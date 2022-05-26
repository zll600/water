#include "serial_task_queue.h"

#include <iostream>

namespace water {

SerialTaskQueue::SerialTaskQueue(const std::string& name)
    : queeu_name_(name),
    thr_(std::bind(&SerialTaskQueue::QueueFunc, this)) {
    if (name.empty()) {
        queeu_name_ = "serial_task_queue_";
    }
    std::cout << "constract SerialTaskQueue(" << queeu_name_ << ")" << std::endl;
}

SerialTaskQueue::~SerialTaskQueue() {
    stop_ = true;
    task_cond_.notify_all();
    thr_.join();
    std::cout << "unconstract SerialTaskQueue(" << queeu_name_ << ")" << std::endl;
}

void SerialTaskQueue::RunTaskInQueue(const std::function<void()>& task) {
    std::unique_lock<std::mutex> lk(task_mutex_);
    task_queue_.push(task);
    task_cond_.notify_one();
}

void SerialTaskQueue::QueueFunc() {
    while (!stop_) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lk(task_mutex_);
            while (!stop_ && task_queue_.size() == 0) {
                task_cond_.wait(lk);
            }

            if (!task_queue_.empty()) {
                std::cout << "got a new task!" << std::endl;
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
