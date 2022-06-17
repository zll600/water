#include "inetaddress.h"

#include <cstring>
#include <cassert>
#include <netdb.h>

#include <thread>

#include "water/base/logger.h"

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

namespace water {

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),"");
//static_assert(offsetof(sockaddr_in, sin_family) == 0,"");
//static_assert(offsetof(sockaddr_in6, sin6_family) == 0,"");
//static_assert(offsetof(sockaddr_in, sin_port) == 2,"");
//static_assert(offsetof(sockaddr_in6, sin6_port) == 2,"");

#if !(__GNUC_PREREQ (4,6))
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
InetAddress::InetAddress(uint16_t port, bool loopback_only, bool ipv6) {
    if (ipv6) {
        bzero(&addr6_, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopback_only ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = ntohs(port);
    } else {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopback_only ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = ntohl(ip);
        addr_.sin_port = ntohs(port);
    }
}

InetAddress::InetAddress(const std::string &ip, uint16_t port, bool ipv6) {
    if (ipv6) {
        bzero(&addr6_, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = htons(port);
        if (::inet_pton(AF_INET6, ip.c_str(), &addr6_.sin6_addr) <= 0) {
            LOG_SYSERR << "sockets::fromIpPort";
        }
    } else {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
            LOG_SYSERR << "sockets::fromIpPort";
        }
    }
}

std::string InetAddress::ToIpPort() const {
    char buf[64] = "";
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf, ":%u", port);
    return ToIp() + std::string(buf);
}

std::string InetAddress::ToIp() const {
    char buf[64];
    if (addr_.sin_family == AF_INET) {
        ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    } else if (addr_.sin_family == AF_INET6) {
        ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, sizeof(buf));
    }

    return buf;
}

uint32_t InetAddress::IpNetEndian() const
{
    assert(Family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::ToPort() const {
    return ntohs(PortNetEndian());
}

static thread_local char thread_resolve_buffer[64 * 1024];

bool InetAddress::resolve(const std::string& hostname, InetAddress* out) {
    assert(out != nullptr);
    struct hostent hent;
    struct hostent* he = nullptr;
    int herrno = 0;
    bzero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, thread_resolve_buffer, sizeof(thread_resolve_buffer), &he, &herrno);
    if (ret == 0 && he != nullptr) {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    } else {
        if (ret) {
            LOG_SYSERR << "InetAddress::resolve";
        }
        return false;
    }
}

}
