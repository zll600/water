#ifndef __WATER_NET_INETADDRESS_H__
#define __WATER_NET_INETADDRESS_H__

#include <cstdint>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>

namespace water {
//namespace sockets
//{
//const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
//}

///
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress {
 public:
    // Constructs an endpoint with given port number.
    // Mostly used in TcpServer listening.
    explicit InetAddress(uint16_t port = 0, bool loopback_only = false, bool ipv6 = false);

    // Constructs an endpoint with given ip and port.
    // @c ip should be "1.2.3.4"
    InetAddress(const std::string &ip, uint16_t port, bool ipv6 = false);

    // Constructs an endpoint with given struct @c sockaddr_in
    // Mostly used when accepting new connections
    explicit InetAddress(const struct sockaddr_in& addr)
    : addr_(addr)
    { }

    explicit InetAddress(const struct sockaddr_in6& addr)
    : addr6_(addr)
    { }

    sa_family_t Family() const { return addr_.sin_family; }
    std::string ToIp() const;
    std::string ToIpPort() const;
    uint16_t ToPort() const;

    // default copy/assignment are Okay
    InetAddress(const InetAddress& addr) =default;
    InetAddress& operator=(const InetAddress& addr) = default;

    const struct sockaddr* get_sock_addr() const {
      return static_cast<const struct sockaddr*>((void *)(&addr6_));
    }
    void set_sock_addr_inet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

    uint32_t IpNetEndian() const;
    uint16_t PortNetEndian() const { return addr_.sin_port; }

    // resolve hostname to IP address, not changing port or sin_family
    // return true on success.
    // thread safe
    static bool resolve(const std::string& hostname, InetAddress* result);
    // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t port = 0);

 private:
  union
  {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}


#endif  // __WATER_NET_INETADDRESS_H__
