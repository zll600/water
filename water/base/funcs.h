#ifndef __WATERT_BASE_FUNCS_H__
#define __WATERT_BASE_FUNCS_H__

#include <cstdint>
#include <arpa/inet.h>

namespace water {

uint64_t hton64(uint64_t port) {
    return (static_cast<uint64_t>(htonl(port)) << 32) | htonl(port >> 32);
}

uint64_t ntoh64(uint64_t port) {
    return (static_cast<uint64_t>(ntohl(port)) << 32) | ntohl(port >> 32);
}

}   // namespace water

#endif // __WATERT_BASE_FUNCS_H__
