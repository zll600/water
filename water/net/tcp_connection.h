#ifndef __WATER_NET_TCPCONNECTION_H__
#define __WATER_NET_TCPCONNECTION_H__

#include <memory>

#include "callbacks.h"
#include "socket.h"
#include "inetaddress.h"
#include "water/base/noncopyable.h"
#include "water/base/msg_buffer.h"

namespace water {

class EventLoop;
class Channel;

class TcpConnection : Noncopyable, public std::enable_shared_from_this<TcpConnection> {
 public:
    TcpConnection(EventLoop *loop, int sock_fd, const InetAddress& local_addr, const InetAddress& peer_addr);
    ~TcpConnection();

    void Send(const char *msg, uint64_t len);
    void Send(const std::string& msg);

    void ConnectionEstablished();
    bool IsConnected() const { return state_ == ConnState::Connected; }
    bool IsDisconnected() const { return state_ == ConnState::Disconnected; }

    void set_recv_msg_callback(const RecvMessageCallback& cb) { recv_msg_callback_ = cb; }
    void set_conn_callback(const ConnectionCallback& cb) { conn_callback_ = cb; }
    // Internal use only
    void set_close_callback(const CloseCallback& cb) { close_callback_ = cb; }
    void ConnectionDestroy();
    void Shutdown();

 private:
    enum ConnState {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting
    };

    EventLoop *loop_;
    std::unique_ptr<Channel> io_channel_ptr_;
    std::unique_ptr<Socket> socket_ptr_;
    MsgBuffer read_buffer_;
    MsgBuffer write_buffer_;
    ConnState state_;

    InetAddress local_addr_, peer_addr_;
    RecvMessageCallback recv_msg_callback_;
    ConnectionCallback conn_callback_;
    CloseCallback close_callback_;

    void ReadCallback();
    void WriteCallback();
    void HandleClose();
    void SendInLoop(const std::string& msg);
};

}   // namespace water

#endif // __WATER_NET_TCPCONNECTION_H__
