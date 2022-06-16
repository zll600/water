#ifndef __WATER_NET_CHANNEL_H__
#define __WATER_NET_CHANNEL_H__

#include "water/base/noncopyable.h"

#include <functional>

namespace water {

class EventLoop;

// A selectable I/O channel
//
// 每个 Channel 对象都只属于某一个 IO 线程
// 自始至终值负责一个文件描述符（fd）
// 但它并不拥有这个 fd，也不会在析构的时候关闭它
class Channel : Noncopyable {
 public:
    using EventCallback = std::function<void()>;
    Channel(EventLoop *loop, int fd);

    /**
     * 处理事件
     */
    void HandleEvent();
    // 设置读回调函数
    void set_read_callback(const EventCallback& cb) {
        read_callback_ = cb;
    }
    // 设置写回调函数
    void set_write_callback(const EventCallback& cb) {
        write_callback_ = cb;
    }

    // 获取文件描述符
    int get_fd() const {
        return fd_;
    }

    // 读取 IO 事件
    int get_events() const {
        return events_;
    }

    // 设置活动事件
    int set_revents(int revt) {
        revents_ = revt;
        return revt;
    }

    // 是否无事件
    bool IsNoneEvent() const {
        return events_ == kNoneEvent;
    }

    // 获取索引
    int get_index() const {
        return index_;
    }
    
    // 设置索引
    void set_index(int index) {
        index_ = index;
    }

    // 设置全部事件不可用
    void DisableAll() {
        events_ = kNoneEvent;
        Update();
    }

    // 从 EventLoop 中移除该 Channel
    void Remove();

    // 获取 loop_ 指针
    EventLoop* OwnerLoop() {
        return loop_;
    }

    void EnableReading() {
        events_ |= kReadEvent;
        Update();
    }

 private:
    // 无事件
    static const int kNoneEvent;
    // 读事件
    static const int kReadEvent;
    // 写事件
    static const int kWriteEvent;

    EventLoop *loop_;   // 所属事件循环
    const int fd_;      // 负责的文件描述符
    int events_;    // IO 事件，由用户设置
    int revents_;   // 活动事件，由 epoll 或者 poll 设置
    int index_;     // 表示当前 channel 的状态
    bool added_to_loop_ = false;

    // 事件回调函数
    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;

    void Update();
};

} // namespace water

#endif // __WATER_NET_CHANNEL_H__
