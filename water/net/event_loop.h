#ifndef __WATER_NET_EVENTLOOP_H__
#define __WATER_NET_EVENTLOOP_H__

#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>

#include "water/base/noncopyable.h"
#include "water/base/date.h"
#include "channel.h"

namespace water {

class Poller;
class TimerQueue;

// Channel 列表类型
using ChannelList = std::vector<Channel*>;
// pending functor 类型
using Func = std::function<void()>;

// IO 线程创建 EventLoop 对象
// EventLoop 的主要功能是运行事件循环 EventLoop::Loop()
// EventLoop 对象的生命周期和 IO 线程一样长，它不必是 heap 对象
class EventLoop : public Noncopyable {
 public:
    EventLoop();
    ~EventLoop();

    // EventLoop::AssertInLoopThread() 和 EventLoop::IsInLoopThread()
    // 为线程安全（可跨线程调用）的函数和只能在特定线程执行的函数（主要是 IO 线程）
    // 提供检查前提条件的功能

    // 调用线程是否为 IO 线程
    bool IsInLoopThread() const {
        return thread_id_ == std::this_thread::get_id();
    }

     /**
      * 断言
      * 如果不是在 IO 线程中调用，则退出
      */
    void AssertInLoopThread() {
        if (!IsInLoopThread()) {
            AbortNotInLoopThread();
        }
    }

    /**
     * 运行事件循环
     * 必须在 IO 线程中调用该函数
     */
    void Loop();

    /**
     * --- 线程安全 ---
     * 退出事件循环
     * 如果通过裸指针调用，非100%线程安全。
     * 如果通过智能指针调用，100%线程安全。
     */
    void set_quit();
    // 更新 Channel
    void UpdateChannel(Channel *chan);
    // 移除 Channel
    void RemoveChannel(Channel *chan);

    /**
     * --- 线程安全 ---
     * 在 IO 线程中调用函数
     * 如果在非 IO 线程中调用该函数，会唤醒事件循环，让事件循环处理
     * 如果在 IO 线程中调用该函数，回调函数直接在该函数中运行
     */
    void RunInLoop(const Func& func);

    /**
     * --- 线程安全 ---
     * 将回调函数放到队列中，并在必要时唤醒 IO 线程
     * 队列中的回调函数在调用完时间回调函数后被调用
     */
    void QueueInLoop(const Func& func);

    /**
     * 内部使用
     * 唤醒 IO 线程，处理
     */
    void WakeUp();
    void WakeUpRead();

    /**
     * --- 线程安全 ---
     * 在指定时间调用 func
     */
    void RunAt(const Date& time, const Func& func);

    /**
     * --- 线程安全 ---
     * 延迟一段时间调用 func
     */
    void RunAfter(double delay, const Func& func);

    /**
     * --- 线程安全 ---
     * 周期调用 func
     */
    void RunEvery(double interval, const Func& cb);

    static EventLoop* GetEventLoopOfCurrentThread();

 private:
    bool looping_;  // 是否在实践循环中
    const std::thread::id thread_id_;   // event_loop 所属线程 id
    std::unique_ptr<Poller> poller_;    // 拥有的 Poller 对象的智能指针

    ChannelList active_channels_;   // 有事件需要处理的 channel 列表
    Channel *current_active_channel_;   // 当前正在处理的 channel
    bool quit_; // 是否退出时间循环
    bool event_handling_;

    std::mutex funcs_mutex_;

    std::vector<Func> funcs_;    // 任务函数列表
    std::unique_ptr<TimerQueue> timer_queue_;   // 定时器队列
    bool calling_funcs_ = false;    // 是否正在出来 pending functor
    int wake_up_fd_;    // 用于唤醒 IO 线程的文件描述符
    std::unique_ptr<Channel> wake_up_channel_ptr_;  // 不需要像内部类 TimerQueue 一样暴露给客户端，不需共享所有权

    /**
     * 如果不在 IO 线程中调用此函数，则打印 IO 线程信息
     */
    void AbortNotInLoopThread();
    void DoRunInLoopFuncs();
};

} // namespace water

#endif // __WATER_NET_EVENTLOOP_H__
