#ifndef __WATER_NET_TCPSERVER_H__
#define __WATER_NET_TCPSERVER_H__

#include <memory>
#include <string>

#include "water/base/noncopyable.h"
#include "event_loop.h"
#include "inetaddress.h"
#include "acceptor.h"

namespace water {

class TcpServer : Noncopyable {
 public:
    TcpServer(EventLoop *loop, const InetAddress& addr, const std::string& name);
    ~TcpServer();
    void Start();

 private:
    EventLoop *loop_;
    std::unique_ptr<Acceptor> acceptor_ptr_;
    std::string server_name_;
    
    void NewConnection(int fd, const InetAddress& peer);
};

}   // namespace water

#endif // __WATER_NET_TCPSERVER_H__

