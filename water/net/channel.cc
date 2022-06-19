#include "channel.h"

#include <cassert>
#include <poll.h>

#include "event_loop.h"
#include "water/base/logger.h"

namespace water {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    tied_(false) {
}

void Channel::Remove() {
    assert(events_ == kNoneEvent);
    added_to_loop_ = false;
    loop_->RemoveChannel(this);
}

void Channel::Update() {
    loop_->UpdateChannel(this);
}

void Channel::HandleEvent() {
    LOG_TRACE << "revents: " << revents_;
    if (tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard) {
            HandleEventSafely();
        }
    } else {
        HandleEventSafely();
    }
}

void Channel::HandleEventSafely() {
    LOG_TRACE << "revents: " << revents_;
    if (revents_ & POLLNVAL) {
        
    }

    if (revents_ & (POLLNVAL | POLLERR)) { if (error_callback_) {
            error_callback_();
        }
    }

    // 可读
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        LOG_TRACE << "handle read";
        if (read_callback_) {
            read_callback_();
        }
    }

    // 可写
    if (revents_ & POLLOUT) {
        if (write_callback_) {
            write_callback_();
        }
    }
}

} // namespace water
