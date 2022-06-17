#include "timer_queue.h"

#include <cstring>
#include <sys/timerfd.h>
#include <unistd.h>

#include <iostream>
#include <functional>
#include <memory>

#include "event_loop.h"

namespace water {

// 创建一个 timer_fd
int CreateTimerfd() {
    int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if (timer_fd < 0) {
        std::cerr << "create timerfd failed!" << std::endl;
    }
    return timer_fd;
}

// 将时间转换为 timespec 格式
struct timespec HowMuchTimeFromNow(const Date& when) {
    int64_t micro_seconds = when.get_micro_seconds_since_epoch() - Date::Now().get_micro_seconds_since_epoch();

    if (micro_seconds < 100) {
        micro_seconds = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(micro_seconds / 1000000);
    ts.tv_nsec = static_cast<long>((micro_seconds % 1000000) * 1000);
    return ts;
}

// 更新定时器，唤醒 IO 线程，处理定时器任务
void ResetTimerFd(int timer_fd, const Date& expiration) {
    // wake up loop by timerfd_settime()
    struct itimerspec new_value;
    struct itimerspec old_value;
    bzero(&new_value, sizeof(new_value));
    bzero(&old_value, sizeof(old_value));
    new_value.it_value = HowMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timer_fd, 0, &new_value, &old_value);
    if (ret) {
        std::cerr << "timerfd_settime()";
    }
}

// 读取 timerfd 的数据
void ReadTimerFd(int timer_fd, const Date& now) {
    uint64_t how_many;
    ssize_t n = ::read(timer_fd, &how_many, sizeof(how_many));
    if (n != sizeof(how_many)) {
        std::cerr << "ReadTimerFd error" << std::endl;
    }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
    timerfd_(CreateTimerfd()),
    timerfd_channel_(loop, timerfd_),
    timers_(),
    calling_expired_timers_(false) {
    // 设置"读"回调函数和可读
    timerfd_channel_.set_read_callback(
            std::bind(&TimerQueue::HandleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime
    timerfd_channel_.EnableReading();
}

TimerQueue::~TimerQueue() {
    timerfd_channel_.DisableAll();
    timerfd_channel_.Remove();
    ::close(timerfd_);
}

void TimerQueue::AddTimer(const TimerCallback& cb, const Date& when, double interval) {
    std::shared_ptr<Timer> timer_ptr = std::make_shared<Timer>(cb, when, interval);
    // 将添加定时器的实际工作转移到 IO 线程，使得不加锁也能保证线程安全性
    loop_->RunInLoop([=]() {
        AddTimerInLoop(timer_ptr);
    });
}

void TimerQueue::AddTimerInLoop(const TimerPtr& timer) {
    loop_->AssertInLoopThread();
    if (Insert(timer)) {
        // 插入的定时器是否是最早到期的定时器
        // 更新定时器文件描述符
        ResetTimerFd(timerfd_, timer->get_when());        
    }
}

bool TimerQueue::Insert(const TimerPtr& timer_ptr) {
    loop_->AssertInLoopThread();
    bool earliest_changed = false;  // 定时器插入后是否为最早到期的定时器
    if (timers_.size() == 0 || *timer_ptr < *timers_.top()) {
        earliest_changed = true;
    }
    timers_.push(timer_ptr);
    return earliest_changed;
}

void TimerQueue::HandleRead() {
    loop_->AssertInLoopThread();
    const Date now = Date::Now();

    ReadTimerFd(timerfd_, now); // // 读取数据，防止重复触发可读事件

    std::vector<TimerPtr> expired_timers = GetExpired(now); // 读取到期定时器

    calling_expired_timers_ = true;

    // safe to callback outside critical section
    for (auto timer_ptr : expired_timers) {
        timer_ptr->Run();
    }
    calling_expired_timers_ = false;

    Reset(expired_timers, now);
}

std::vector<TimerPtr> TimerQueue::GetExpired(const Date& now) {
    std::vector<TimerPtr> expired;
    while (!timers_.empty()) {
        if (timers_.top()->get_when() < now) {
            expired.push_back(timers_.top());
            timers_.pop();
        } else {
            break;
        }
    }
    return expired;
}

void TimerQueue::Reset(const std::vector<TimerPtr>& expired, const Date& now) {
    for (auto timer_ptr : expired) {
        if (timer_ptr->IsRepeat()) {
            timer_ptr->Restart(now);
            Insert(timer_ptr);
        }
    }

    // 更新 timerfd 到期时间
    if (!timers_.empty()) {
        const Date next_expire = timers_.top()->get_when();
        ResetTimerFd(timerfd_, next_expire);
    }
}

}   // namespace water
