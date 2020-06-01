//
// Created by andy on 20-5-23.
//

#include "InetAddress.h"

#include <netinet/in.h>
#include <strings.h>

#include "net/SocketsOps.h"

using namespace reactor;
using namespace reactor::net;
static const in_addr_t KInaddrAny = INADDR_ANY;
/*
 struct sockaddr_in6
  {
    __SOCKADDR_COMMON (sin6_);
    in_port_t sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
    };
 */
/*
struct sockaddr_in
{
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;
    struct in_addr sin_addr;
    unsigned char sin_zero[sizeof (struct sockaddr)
                           - __SOCKADDR_COMMON_SIZE
                           - sizeof (in_port_t)
                           - sizeof (struct in_addr)];
};*/
// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"
static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "sockaddr_in6 size not equal InetAddress");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
    static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
    static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
    if (ipv6) {
        bzero(&addr6_, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = sockets::hostToNetwork16(port);
        addr6_.sin6_addr = loopbackOnly ? in6addr_loopback : in6addr_any;

    } else {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
    }
}

InetAddress::InetAddress(const StringArg &ip, uint16_t port, bool ipv6) {
    if (ipv6) {
        bzero(&addr6_,sizeof(addr6_));
        sockets::fromIpPort(ip.c_str(),port,&addr6_);
    } else {
        bzero(&addr_, sizeof(addr_));
        sockets::fromIpPort(ip.c_str(), port, &addr_);
    }
}
std::string InetAddress:: toIpPort() const{
    char buf[64]="";
    sockets::toIpPort(buf,sizeof buf,getSockAddrInet());
    return buf;
}


