#ifndef __WATER_NET_SOCKET_H__
#define __WATER_NET_SOCKET_H__

#include "water/base/noncopyable.h"
#include "inetaddress.h"

namespace water {

class Socket : Noncopyable {
 public:
    Socket(int family);
    ~Socket();

    // abort if address in use
    void BindAddress(const InetAddress& localaddr);
    // abort if address in use
    void Listen();
    int Accept(InetAddress *peer_addr);
    void CloseWrite();
    int get_fd() const { return sock_fd_; }
 protected:
    int sock_fd_;
};

}   // namespace water

#endif // __WATER_NET_SOCKET_H__
