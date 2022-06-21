#ifndef __WATER_NET_EVENTLOOPTHREAD_H__
#define __WATER_NET_EVENTLOOPTHREAD_H__

#include <mutex>
#include <condition_variable>

#include "event_loop.h"
#include "water/base/noncopyable.h"
#include "water/base/serial_task_queue.h"

namespace water {

class EventLoopThread : Noncopyable {
 public:
    EventLoopThread();
    ~EventLoopThread();

    void Run();
    void Stop();
    void Wait();

    EventLoop* get_loop() { return loop_; }

 private:
    EventLoop *loop_;
    SerialTaskQueue loop_queue_;
    std::mutex mut_;
    std::condition_variable cond_;

    void LoopFuncs();
};

}   // namespace water

#endif // __WATER_NET_EVENTLOOPTHREAD_H__
