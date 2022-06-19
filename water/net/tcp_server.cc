#include "tcp_server.h"

#include <unistd.h>

#include <functional>
#include <vector>

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

void TcpServer::NewConnection(int sock_fd, const InetAddress& peer_addr) {
    LOG_TRACE << "new connection: fd = " << sock_fd << " address = " << peer_addr.ToIpPort();
    //test code for blocking or nonblocking
    std::vector<char> str(1024*1024*100);
    for(size_t i = 0; i < str.size(); i++)
        str[i] = 'A';
    LOG_TRACE << "vector size:" << str.size();
    size_t n = ::write(sock_fd, &str[0], str.size());
    LOG_TRACE<<"write "<<n<<" bytes";
    TcpConnectionPtr new_conn = std::make_shared<TcpConnection>(loop_, sock_fd, acceptor_ptr_->get_addr(), peer_addr);
    new_conn->set_recv_msg_callback([=](const TcpConnectionPtr& connectionPtr, MsgBuffer *buffer) {
        LOG_TRACE << "recv Msg "<< buffer->ReadableBytes() << " bytes";
        if(recv_msg_callback_)
            recv_msg_callback_(connectionPtr, buffer);
    });
    conn_set_.insert(new_conn);
}

}   // namespace water
