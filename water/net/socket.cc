#include "socket.h"

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "water/base/logger.h"

namespace water {

Socket::Socket(int sock_fd)
    : sock_fd_(sock_fd) {
}

Socket::~Socket() {
    if (sock_fd_ >= 0) {
        LOG_TRACE << "socket destructed";
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

int Socket::Read(char *buffer, uint64_t len) {
    return ::read(sock_fd_, buffer, len);
}

int Socket::CreateNonblockingSocketOrDie(int family) {
    int sock_fd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sock_fd < 0) {
        LOG_SYSERR << "sockets::CreateNonblockingOrDie";
        exit(-1);
    }
    return sock_fd;
}

void SetNoncBlockAndCloseExec(int sock_fd) {
    // non-block
    int flags = ::fcntl(sock_fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sock_fd, F_SETFL, flags);
    // FIXME check

    // close-on-exec
    flags = ::fcntl(sock_fd, F_GETFL, 0);
    flags |= O_CLOEXEC;
    ret = ::fcntl(sock_fd, F_SETFL, flags);
    // FIXME check
    
    (void)ret;
}

struct sockaddr_in6 Socket::GetLocalAddr(int sockfd) {
    struct sockaddr_in6 local_addr;
    bzero(&local_addr, sizeof(local_addr));
    socklen_t addr_len = static_cast<socklen_t>(sizeof(local_addr));
    if (::getsockname(sockfd, static_cast<struct sockaddr*>((void *)(&local_addr)), &addr_len) < 0) {
        LOG_SYSERR << "sockets::getLocalAddr";
    }
    return local_addr;
}

struct sockaddr_in6 Socket::GetPeerAddr(int sockfd) {
    struct sockaddr_in6 peer_addr;
    bzero(&peer_addr, sizeof(peer_addr));
    socklen_t addr_len = static_cast<socklen_t>(sizeof(peer_addr));
    if (::getpeername(sockfd, static_cast<struct sockaddr*>((void *)(&peer_addr)), &addr_len) < 0) {
        LOG_SYSERR << "sockets::getPeerAddr";
    }
    return peer_addr;
}

}   // namespace water
