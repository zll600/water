#include "acceptor.h"

#include <unistd.h>

#include "water/base/logger.h"

namespace water {

Acceptor::Acceptor(EventLoop *loop, const InetAddress& addr) 
        : sock_(addr.get_sock_addr()->sa_family),
        addr_(addr), 
        loop_(loop),
        accept_channel_(loop, sock_.get_fd()) {
    sock_.BindAddress(addr);
    accept_channel_.set_read_callback(std::bind(&Acceptor::ReadCallback, this));
}

Acceptor::~Acceptor() {
    accept_channel_.DisableAll();
    accept_channel_.Remove();
}

void Acceptor::Listen() {
    loop_->AssertInLoopThread();
    sock_.Listen();
    accept_channel_.EnableReading();
}

void Acceptor::ReadCallback() {
    InetAddress peer;
    int new_sock = sock_.Accept(&peer);
    if (new_sock >= 0) {
        if (new_connection_callback_) {
            new_connection_callback_(new_sock, peer);
        } else {
            close(new_sock);
        }
    } else {
        LOG_SYSERR << "Acceptor::ReadCallback";
    }
}

}   // namespace water
