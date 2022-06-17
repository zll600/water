#include "socket.h"

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include "water/base/logger.h"

namespace water {

Socket::Socket(int family) {
    sock_fd_ = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sock_fd_ < 0) {
        LOG_SYSERR << "sockets::createNonblockingOrDie";
        exit(-1);
    }
}

Socket::~Socket() {
    if (sock_fd_ >= 0) {
        close(sock_fd_);
    }
}

void Socket::BindAddress(const InetAddress& local_addr) {
    assert(sock_fd_ > 0);
    int ret = ::bind(sock_fd_, local_addr.get_sock_addr(), sizeof(sockaddr));
    if (ret != 0) {
        LOG_SYSERR << "bind failed";
        exit(-1);
    }
}

void Socket::Listen() {
    assert(sock_fd_);
    int ret = ::listen(sock_fd_, SOMAXCONN);
    if (ret < 0) {
        LOG_SYSERR << "Listen failed";
    }
}

int Socket::Accept(InetAddress *peeraddr) {
    struct sockaddr_in6 addr6;
    bzero(&addr6, sizeof(addr6));
    socklen_t size = sizeof(addr6);
    int connfd = ::accept(sock_fd_, (struct sockaddr*)&addr6, &size);
    if (connfd >= 0) {
        peeraddr->set_sock_addr_inet6(addr6);
    }
    return connfd;
}

void Socket::CloseWrite() {
    if (::shutdown(sock_fd_, SHUT_WR) < 0) {
        LOG_SYSERR << "sockets::shutdownWrite";
    }
}

}   // namespace water
