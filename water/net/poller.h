#ifndef __WATER_NET_POLLER_H__
#define __WATER_NET_POLLER_H__

#include <sys/epoll.h>

#include <vector>
#include <map>

#include "water/base/noncopyable.h"
#include "event_loop.h"

namespace water {

class Channel;
using EventList = std::vector<struct epoll_event>;

// Poller 的职责：
    // 1 管理（增删改） Channel 列表：有关心事件的 Channel 和 无关心事件的 Channel
    // 2 Poller::Poll()，监听文件描述符（有关心事件的 Channel 对应的文件描述符）
    //   当有事件发生时，将"活跃"的 Channel 填充到 active_channels_ 中，
    //   供 EventLoop 处理相应的事件，即调用 Channel::HandleEvent()。
    //
    // Poller 是 EventLoop 对象的间接成员，
    // 只供 owner EventLoop 在 IO 线程调用，因此无须加锁。
    // 其生命周期与 EVentLoop 一样长
    //
    // Poller 采用的是 level trigger
class Poller : public Noncopyable {
 public:
    Poller(EventLoop *loop);
    ~Poller();

    void AssertInLoopThread() {
        owner_loop_->AssertInLoopThread();
    }

    // 只能在 IO 线程中调用
    //  监听文件描述符
    void Poll(int timeout_ms, ChannelList *active_channels);

    /**
     * --- 只能在 IO 线程中调用 ---
     * 更新 Channel
     */
    void UpdateChannel(Channel *chan);
    /**
     * --- 只能在 IO 线程中调用 ---
     * 移除 Channel
     */
    void RemoveChannel(Channel *chan);

 private:
    using ChannelMap = std::map<int, Channel*>;

    EventLoop *owner_loop_; // 所属事件循环
    static const int kInitEventListSize = 16;   // 事件数组大小
    int epoll_fd_;  // epoll 文件描述符
    EventList events_;  // 活动文件描述符列表
    ChannelMap channels_;   // <活动文件描述福,Channel>

    void Update(int operation, Channel *channel);
    void FillactiveChannels(int num_events, ChannelList *active_channels) const;
};

} // namesapce water

#endif // __WATER_NET_POLLER_H__
