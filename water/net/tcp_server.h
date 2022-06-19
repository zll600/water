#ifndef __WATER_NET_TCPSERVER_H__
#define __WATER_NET_TCPSERVER_H__

#include <memory>
#include <string>
#include <set>

#include "water/base/noncopyable.h"
#include "event_loop.h"
#include "inetaddress.h"
#include "acceptor.h"
#include "callbacks.h"
#include "tcp_connection.h"

namespace water {

class TcpServer : Noncopyable {
 public:
    TcpServer(EventLoop *loop, const InetAddress& addr, const std::string& name);
    ~TcpServer();
    void Start();
    void set_recv_msg_callback(const RecvMessageCallback& cb) { recv_msg_callback_ = cb; }
    void set_conn_callback(const ConnectionCallback& cb) { conn_callback_ = cb; }

 private:
    EventLoop *loop_;
    std::unique_ptr<Acceptor> acceptor_ptr_;
    std::string server_name_;
    std::set<TcpConnectionPtr> conn_set_;
    RecvMessageCallback recv_msg_callback_;
    ConnectionCallback conn_callback_;
    
    void NewConnection(int fd, const InetAddress& peer);
};

}   // namespace water

#endif // __WATER_NET_TCPSERVER_H__

