#include "tcp_connection.h"

#include <cassert>
#include <unistd.h>

#include "channel.h"
#include "event_loop.h"
#include "water/base/logger.h"

#define FETCH_SIZE 2048
#define SEND_ORDER 1

namespace water {

TcpConnection::TcpConnection(EventLoop *loop, int sock_fd, const InetAddress& local_addr,
                             const InetAddress& peer_addr):
        loop_(loop),
        io_channel_ptr_(new Channel(loop, sock_fd)),
        socket_ptr_(new Socket(sock_fd)),
        local_addr_(local_addr),
        peer_addr_(peer_addr) {
    LOG_TRACE << "new connection:" << peer_addr.ToIpPort() << "->" << local_addr.ToIpPort();
    io_channel_ptr_->set_read_callback(std::bind(&TcpConnection::ReadCallback,this));
    io_channel_ptr_->set_write_callback(std::bind(&TcpConnection::WriteCallback, this));
    io_channel_ptr_->EnableReading();
}

TcpConnection::~TcpConnection() {
    LOG_TRACE << "connection closed:" << peer_addr_.ToIpPort() << "->" << local_addr_.ToIpPort();
}

void TcpConnection::ReadCallback() {
    LOG_TRACE<<"read Callback";
    loop_->AssertInLoopThread();
    int ret = 0;

    size_t n = read_buffer_.ReadFd(socket_ptr_->get_fd(), &ret);
    LOG_TRACE<<"read "<<n<<" bytes from socket";
    if(n == 0) {
        //socket closed by peer
        HandleClose();
    } else if(n<0) {
        LOG_SYSERR<<"read socket error";
    }

    if(n > 0 && recv_msg_callback_) {
        recv_msg_callback_(shared_from_this(), &read_buffer_);
    }
} 

void TcpConnection::WriteCallback() {
    loop_->AssertInLoopThread();
    if (io_channel_ptr_->IsWriting()) {
        if(write_buffer_.ReadableBytes() <= 0) {
            io_channel_ptr_->DisableWriting();
            // FIXME add write complete callback
            if (state_ == ConnState::Disconnecting) {
                socket_ptr_->CloseWrite();
            }
        } else {
            size_t n = ::write(socket_ptr_->get_fd(), write_buffer_.Peek(), write_buffer_.ReadableBytes());
            write_buffer_.Retrieve(n);
            // if(write_buffer_.ReadableBytes() == 0) {
            //     io_channel_ptr_->DisableWriting();
            //  }
        }
    } else {
        LOG_SYSERR << "no writing but call write callback";
    }
}

void TcpConnection::ConnectionEstablished() {
    loop_->AssertInLoopThread();
    assert(state_ == ConnState::Connecting);
    io_channel_ptr_->Tie(shared_from_this());
    io_channel_ptr_->EnableReading();
    state_ = ConnState::Connected;
    if (conn_callback_) {
        conn_callback_(shared_from_this());
    }
}

void TcpConnection::HandleClose() {
    LOG_TRACE<<"connection closed";
    loop_->AssertInLoopThread();
    state_ = Disconnected;
    io_channel_ptr_->DisableAll();
    auto guard_this = shared_from_this();
    if(conn_callback_)
        conn_callback_(guard_this);
    if(close_callback_) {
        LOG_TRACE<<"to call close callback";
        close_callback_(guard_this);
    }
}

void TcpConnection::ConnectionDestroy() {
    loop_->AssertInLoopThread();
    if (state_ == Connected) {
        state_ = Disconnected;
        io_channel_ptr_->DisableAll();

        conn_callback_(shared_from_this());
    }
    io_channel_ptr_->Remove();
}

void TcpConnection::Shutdown() {
    loop_->RunInLoop([=] {
        if (state_ == ConnState::Connected) {
            state_ = ConnState::Disconnecting;
            if (!io_channel_ptr_->IsWriting()) {
                socket_ptr_->CloseWrite();
            }
        }
    });
}

void TcpConnection::SendInLoop(const std::string& msg) {
    loop_->AssertInLoopThread();
    write_buffer_.Append(msg);
    if (!io_channel_ptr_->IsWriting()) {
        io_channel_ptr_->EnableWriting();
    }
}

void TcpConnection::Send(const char *msg, uint64_t len) {
    Send(std::move(std::string(msg, len)));
}

void TcpConnection::Send(const std::string& msg){
#if SEND_ORDER
    loop_->RunInLoop([=](){
        SendInLoop(msg);
    });
#else
    if(loop_->IsInLoopThread()) 
        sendInLoop(msg);
    } else {
        loop_->RunInLoop([=](){
            SendInLoop(msg);
        });
    }
#endif
}

}   // namespace water
