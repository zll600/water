#include "event_loop_thread.h"

namespace water {

EventLoopThread::EventLoopThread() 
    : loop_(nullptr),
    loop_queue_("EventLoopThread") {
}

EventLoopThread::~EventLoopThread() {
    if (loop_) {
        loop_->set_quit();
    }
}

void EventLoopThread::Run() {
    loop_queue_.RunTaskInQueue(std::bind(&EventLoopThread::LoopFuncs, this));
    std::unique_lock<std::mutex> lk(mut_);
    while (loop_ == nullptr) {
        cond_.wait(lk);
    }
}

void EventLoopThread::Stop() {
    if (loop_) {
        loop_->set_quit();
    }
}

void EventLoopThread::Wait() {
    loop_queue_.WaitAllTaskFinished();
}

void EventLoopThread::LoopFuncs() {
    EventLoop loop;
    {
        std::lock_guard<std::mutex> lk(mut_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.Loop();
}


}   // namespace water
