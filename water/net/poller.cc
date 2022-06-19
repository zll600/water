#include "poller.h"

#include <cerrno>
#include <cassert>
#include <cstring>
#include <poll.h>
#include <unistd.h>

#include <iostream>

#include "channel.h"
#include "water/base/logger.h"

namespace water {

static_assert(EPOLLIN == POLLIN, "EPOLLIN != POLLIN");
static_assert(EPOLLPRI == POLLPRI, "EPOLLPRI != POLLPRI");
static_assert(EPOLLOUT == POLLOUT, "EPOLLOUT != POLLOUT");
static_assert(EPOLLRDHUP == POLLRDHUP, "EPOLLRDHUP != POLLRDHUP");
static_assert(EPOLLERR == POLLERR, "EPOLLERR != POLLERR");
static_assert(EPOLLHUP == POLLHUP, "EPOLLHUP != POLLHUP");

namespace {
    // channel 的状态
    // 表示还没添加到 ChannelMap 中
    const int kNew = -1;
    // 已添加到 ChannelMap 中
    const int kAdded = 1;
    // 无关心事件，已从 epoll 中删除相应文件描述符，但 ChannelMap 中有记录
    const int kDeleted = 2;
}

Poller::Poller(EventLoop *loop)
    : owner_loop_(loop),
    epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize) {
}

Poller::~Poller() {
    close(epoll_fd_);
}

void Poller::Poll(int timeout_ms, ChannelList *active_channels) {
    // 往 events_ 的内存里写入活跃的事件。
    // events_.data() 返回 vector 底层数组的指针，
    // 也可以通过 &*event_.begin() 获取
    int num_events = ::epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), timeout_ms);
    int save_errno = errno;
    if (num_events > 0) {
        LOG_TRACE << num_events << " events happened ";
        FillactiveChannels(num_events, active_channels);
        if (static_cast<size_t>(num_events) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (num_events == 0) {
        std::cout << "nothing happened" << std::endl;
    } else {
        // error happens
        if (save_errno != EINTR) {
            errno = save_errno;
            std::cout << "Poller::Poll() " << std::endl;
        }
    }
    LOG_TRACE << "active Channels num:"<< active_channels->size();
}

void Poller::FillactiveChannels(int num_events, ChannelList *active_channels) const {
    assert(static_cast<size_t>(num_events) <= events_.size());
    for (int i = 0; i < num_events; ++i) {
        Channel *chan = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = chan->get_fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == chan);
#endif
        chan->set_revents(events_[i].events);
        active_channels->push_back(chan);
    }
}

void Poller::UpdateChannel(Channel *chan) {
    AssertInLoopThread();
    const int index = chan->get_index();
    if (index == kNew || index == kDeleted) {
        // a  new one, add with EPOLL_CTL_ADD
        int fd = chan->get_fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = chan;
        } else {  // index == kDeleted
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == chan);
        }

        chan->set_index(kAdded);
        Update(EPOLL_CTL_ADD, chan);
    } else {
        // update existing one with EPOLL_CTL_MOD
        int fd = chan->get_fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == chan);
        assert(index == kAdded);

        if (chan->IsNoneEvent()) {
            Update(EPOLL_CTL_DEL, chan);
            chan->set_index(kDeleted);
        } else {
            Update(EPOLL_CTL_MOD, chan);
        }
    }
}

void Poller::RemoveChannel(Channel *chan) {
    AssertInLoopThread();
    int fd = chan->get_fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == chan);
    int index = chan->get_index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == kAdded) {
        Update(EPOLL_CTL_DEL, chan);
    }
    chan->set_index(kNew);
}

void Poller::Update(int operation, Channel *channel) {
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->get_events();
    event.data.ptr = channel;
    int fd = channel->get_fd();

    if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            std::cout << "epoll_ctl_del" << std::endl;
        } else {
            std::cout << "epoll_ctl_other" << std::endl;
        }
    }
}

} // namespace water
