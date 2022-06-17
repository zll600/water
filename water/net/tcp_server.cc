#include "tcp_server.h"

#include <functional>

#include "water/base/logger.h"

namespace water {

TcpServer::TcpServer(EventLoop *loop, const InetAddress& addr, const std::string& name)
    : loop_(loop),
    acceptor_ptr_(new Acceptor(loop, addr)),
    server_name_(name) {
    acceptor_ptr_->set_new_connection_callback(
            std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    loop_->AssertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << server_name_ << "] destructing";
}

void TcpServer::Start() {
    loop_->RunInLoop([=]() {
        acceptor_ptr_->Listen();
    });
}

void TcpServer::NewConnection(int sock_fd, const InetAddress& peer) {
    LOG_TRACE << "new connection: fd = " << sock_fd << " address = " << peer.ToIpPort();
}

}   // namespace water
