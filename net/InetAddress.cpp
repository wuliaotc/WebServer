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
static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in),
              "sockaddr_in size not equal InetAddress");

InetAddress::InetAddress(uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string &ip, uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    sockets::fromHostPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::toHostPort() const {
    char buf[32];
    sockets::toHostPort(buf, sizeof(buf), addr_);
    return buf;
}

