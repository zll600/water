#include "serial_task_queue.h"

#include <sys/prctl.h>

#include <iostream>

#include "water/base/logger.h"

namespace water {

SerialTaskQueue::SerialTaskQueue(const std::string& name)
    : queue_name_(name),
    thr_(std::bind(&SerialTaskQueue::QueueFunc, this)) {
    if (name.empty()) {
        queue_name_ = "SerialTaskQueue";
    }
    LOG_TRACE << "construct SerialTaskQueue(" << queue_name_ << ")";
}

SerialTaskQueue::~SerialTaskQueue() {
    stop_ = true;
    task_cond_.notify_all();
    thr_.join();
    LOG_TRACE << "destruct SerialTaskQueue(" << queue_name_ << ")";
}

void SerialTaskQueue::RunTaskInQueue(const std::function<void()>& task) {
    std::lock_guard<std::mutex> lk(task_mutex_);
    task_queue_.push(task);
    task_cond_.notify_one();
}

void SerialTaskQueue::QueueFunc() {
    ::prctl(PR_SET_NAME, queue_name_.c_str());
    while (!stop_) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lk(task_mutex_);
            while (!stop_ && task_queue_.empty()) {
                task_cond_.wait(lk);
            }

            if (!task_queue_.empty()) {
                LOG_TRACE << "get a new task!";
                task = std::move(task_queue_.front());
                task_queue_.pop();
            } else {
                continue;
            }
        }
        task();
    }
}

void SerialTaskQueue::WaitAllTaskFinished() {
    SyncTaskInQueue([](){

    });
}

}   // namespace water
