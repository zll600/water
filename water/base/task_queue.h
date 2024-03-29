#ifndef __WATER_BASE_TASKQUEUE_H__
#define __WATER_BASE_TASKQUEUE_H__

#include <functional>
#include <future>

#include "noncopyable.h"

namespace water {

// 任务队列
class TaskQueue : Noncopyable {
 public:
    virtual void RunTaskInQueue(const std::function<void()>& task) =0;
    virtual std::string get_queue_name() const { return ""; }
    void SyncTaskInQueue(const std::function<void()>& task) {
        std::promise<int> prom;
        std::future<int> fut = prom.get_future();
        RunTaskInQueue([&]() {
            task();
            prom.set_value(1);
        });
        fut.get();
    }
};

} // namespace water

#endif // __WATER_BASE_TASKQUEUE_H__
