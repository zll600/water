#ifndef __WATER_NET_TIMERQUEUE_H__
#define __WATER_NET_TIMERQUEUE_H__

#include <vector>
#include <queue>
#include <memory>

#include "water/base/noncopyable.h"
#include "water/base/date.h"
#include "timer.h"
#include "channel.h"

namespace water {

using TimerPtr = std::shared_ptr<Timer>;

struct cmp {
    bool operator()(const TimerPtr& lhs, const TimerPtr& rhs) {
        return *lhs > *rhs;
    }
};

class EventLoop;
class TimerQueue : public Noncopyable {
 public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    // 添加定时器
    void AddTimer(const TimerCallback& cb, const Date& when, double interval);

    /**
     * 在 EventLoop 中往定时器队列添加定时器
     */
    void AddTimerInLoop(const TimerPtr& timer);

 private:
    EventLoop *loop_;   // 所属时间循环
    const int timerfd_; // 时间事件描述符
    Channel timerfd_channel_;  // timerfs 的 channel
    std::priority_queue<TimerPtr, std::vector<TimerPtr>, cmp> timers_;  // 存储定时器，使用小顶堆
    bool calling_expired_timers_;   // 是否处理到期的定时器
    
    // 定时器到期，timerfd 可写，则会唤醒 IO 线程，在 IO 线程中，处理到期的定时器
    void HandleRead();

    // 插入定时器
    bool Insert(const TimerPtr& timer_ptr);

    /**
     * 获取所有到期定时器，并从定时器队列中移除定时器
     */
    std::vector<TimerPtr> GetExpired();

    /**
     * 将存储到期定时器的定时器队列中的周期定时器更新到期时间，并重新添加到定时器队列中。
     * 同时，更新 timerfd 的时间
     */
    void Reset(const std::vector<TimerPtr>& expired, const Date& now);

    std::vector<TimerPtr> GetExpired(const Date& now);
};

} // namespace water

#endif // __WATER_NET_TIMERRUEUE_H__
