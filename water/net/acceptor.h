#ifndef __WATER_NET_ACCEPTOR_H__
#define __WATER_NET_ACCEPTOR_H__

#include <functional>

#include "inetaddress.h"
#include "socket.h"
#include "event_loop.h"
#include "channel.h"

namespace water {

using NewConnectionCallback = std::function<void(int fd, const InetAddress&)>;

class Acceptor {
 public:
    Acceptor(EventLoop *loop, const InetAddress& addr);
    ~Acceptor();

    const InetAddress& get_addr() const { return addr_; }
    void set_new_connection_callback(const NewConnectionCallback& cb) { new_connection_callback_ = cb; }
    void Listen();

 private:
    Socket sock_;
    InetAddress addr_;
    EventLoop *loop_;
    NewConnectionCallback new_connection_callback_;
    Channel accept_channel_;

    void ReadCallback();
};

}   // namespace water

#endif // __WATER_NET_ACCEPTOR_H__
