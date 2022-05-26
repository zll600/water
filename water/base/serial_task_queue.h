#ifndef __WATER_SERIALTASKQUEUE_H__
#define __WATER_SERIALTASKQUEUE_H__

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "task_queue.h"

namespace water {

class SerialTaskQueue : public TaskQueue {
 public:
    SerialTaskQueue() =delete;
    SerialTaskQueue(const std::string& name = std::string{});
    ~SerialTaskQueue();

    virtual void RunTaskInQueue(const std::function<void()>& task);
    virtual std::string get_queue_name() const { return queeu_name_; }
    void WaitAllTaskFinished();
 private:
    std::string queeu_name_;    // 人物队列名称
    std::queue<std::function<void()>> task_queue_;  // 任务队列
    std::mutex task_mutex_; // 互斥锁
    std::condition_variable task_cond_; // 条件变量
    bool stop_ = false;
    std::thread thr_;   // 线程
    
    void QueueFunc();
};

}   // namespace water

#endif // __WATER_SERIALTASKQUEUE_H__
