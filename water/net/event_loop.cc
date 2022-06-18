#include "event_loop.h"

#include <cstdlib>
#include <cassert>
#include <sys/eventfd.h>
#include <unistd.h>

#include <iostream>
#include <algorithm>

#include "timer_queue.h"
#include "poller.h"
#include "channel.h"
#include "water/base/logger.h"

namespace water {

// 创建时间通知描述符，用于唤醒 IO 线程
int CreateEventfd() {
    int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd < 0) {
        std::cout << "Failed in eventfd" << std::endl;
        abort();
    }
    return event_fd;
}

thread_local EventLoop *t_loop_in_this_thread = nullptr;

const int kPollTimeMs = 10000;
EventLoop::EventLoop()
    : looping_(false),
    thread_id_(std::this_thread::get_id()),
    poller_(new Poller(this)),
    current_active_channel_(nullptr),
    quit_(false),
    event_handling_(false),
    timer_queue_(new TimerQueue(this)),
    wake_up_fd_(CreateEventfd()),
    wake_up_channel_ptr_(new Channel(this, wake_up_fd_)) {
    assert(t_loop_in_this_thread == 0);;
    t_loop_in_this_thread = this;

    wake_up_channel_ptr_->set_read_callback(
            std::bind(&EventLoop::WakeUpRead, this)); 
    wake_up_channel_ptr_->EnableReading();
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loop_in_this_thread = nullptr;
}

EventLoop* EventLoop::GetEventLoopOfCurrentThread() {
    return t_loop_in_this_thread;
}

void EventLoop::UpdateChannel(Channel *chan) {
    assert(chan->OwnerLoop() == this);
    AssertInLoopThread();
    poller_->UpdateChannel(chan);
}

void EventLoop::RemoveChannel(Channel *chan) {
    assert(chan->OwnerLoop() == this);
    AssertInLoopThread();
    if (event_handling_) {
        assert(current_active_channel_ == chan
                || std::find(active_channels_.begin(), active_channels_.end(), chan) == active_channels_.end());
    }
    poller_->RemoveChannel(chan);
}

void EventLoop::set_quit() {
    // EventLoop::quit() 不会让事件循环立即退出，
    // EventLoop::loop() 中事件循环会在下一次检查 while(!quit_) 时退出
    quit_ = true;
}

void EventLoop::Loop() {
    assert(!looping_);
    AssertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        active_channels_.clear();
        poller_->Poll(kPollTimeMs, &active_channels_);

        event_handling_ = true;
        for (auto it = active_channels_.begin();
                it != active_channels_.end(); ++it) {
            current_active_channel_ = *it;
            current_active_channel_->HandleEvent();
        }
        current_active_channel_ = nullptr;
        event_handling_ = false;
        std::cout << "looping_" << std::endl;
        DoRunInLoopFuncs();
    }
    looping_ = false;
}

void EventLoop::AbortNotInLoopThread() {
    exit(-1);
}

void EventLoop::RunInLoop(const Func& func) {
    // 如果在 IO 线程中调用该函数，则直接调用回调函数；
    // 否则，加入队列中
    if (IsInLoopThread()) {
        func();
    } else {
        QueueInLoop(func);
    }
}

void EventLoop::QueueInLoop(const Func& func) {
    {
        std::lock_guard<std::mutex> lk(funcs_mutex_);
        funcs_.push_back(func);
    }

     // 有两种情况需要唤醒 IO 线程：
    // 1. 在非 IO 线程调用 EventLoop::QueueInLoop()；
    // 2. 在 IO 线程调用 EventLoop::QueueInLoop()，但此时正在调用 EventLoop::doPendingFunctors()。
    //    实质上，EventLoop::QueueInLoop() 就是在其中一个 pending functor 中被调用了。
    //    根据 EventLoop::doPendingFunctors() 的实现，doPendingFunctors() 函数被调用时，
    //    为了缩小临界区，会将 pendingFunctors_ 数组 swap 到一个临时数组中，再处理。
    //    那此时加入队列的函数是不会在此次处理 pending functor 中被调用。
    //    所以，为了尽快调用该 cb 函数，则需要调用 EventLoop::wakeup()，在下次循环中立即唤醒 IO 线程。
    //
    // 总结，只有在 IO 线程的事件回调函数中调用 EventLoop::queueInLoop()
    // 才无须调用 EventLoop::wakeup() 唤醒 IO 线程。因为在事件回调处理完之后，
    // 会调用 doPendingFunctors() 函数，处理 pending functor，该 cb  函数也会被调用。
    if (!IsInLoopThread() || calling_funcs_ || !looping_) {
        WakeUp();
    }
}

void EventLoop::RunAt(const Date& time, const Func& func) {
    timer_queue_->AddTimer(func, time, 0);
}

void EventLoop::RunAfter(double delay, const Func& func) {
    RunAt(Date::Now().After(delay), func);
}

void EventLoop::RunEvery(double interval, const Func& func) {
    timer_queue_->AddTimer(func, Date::Now(), interval);
}

void EventLoop::DoRunInLoopFuncs() {
    calling_funcs_ = true;
    std::vector<Func> tmp_funcs;
    {
        std::lock_guard<std::mutex> lk(funcs_mutex_);
        tmp_funcs.swap(funcs_);
    }
    for (auto funcs : tmp_funcs) {
        funcs();
    }
    calling_funcs_ = false;
}

void EventLoop::WakeUp() {
    uint64_t tmp = 1;
    int ret = ::write(wake_up_fd_, &tmp, sizeof(tmp));
    if (ret < 0) {
        LOG_SYSERR << "wake up error";
    }
}

void EventLoop::WakeUpRead() {
    uint64_t tmp;
    int ret = ::read(wake_up_fd_, &tmp, sizeof(tmp));
    if (ret < 0) {
        LOG_SYSERR << "wake up read error";
    }
}

} // namespace water
