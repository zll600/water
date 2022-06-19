#ifndef __WATER_NET_SOCKET_H__
#define __WATER_NET_SOCKET_H__

#include "water/base/noncopyable.h"
#include "inetaddress.h"

namespace water {

class Socket : Noncopyable {
 public:
    explicit Socket(int sock_fd);
    ~Socket();

    // abort if address in use
    void BindAddress(const InetAddress& localaddr);
    // abort if address in use
    void Listen();
    int Accept(InetAddress *peer_addr);
    void CloseWrite();
    int Read(char *buffer, uint64_t len);
    int get_fd() const { return sock_fd_; }


    static int CreateNonblockingSocketOrDie(int family);
    static void SetNoncBlockAndCloseExec(int sock_fd);
    static struct sockaddr_in6 GetLocalAddr(int sockfd);
    static struct sockaddr_in6 GetPeerAddr(int sockfd);
 protected:
    int sock_fd_;
};

}   // namespace water

#endif // __WATER_NET_SOCKET_H__
