//
// Created by andy on 20-5-23.
//
#include "net/Socket.h"
#include "net/InetAddress.h"
#include "net/SocketsOps.h"

#include <strings.h>
#include <netinet/tcp.h>

using namespace reactor;
using namespace reactor::net;
Socket::~Socket() { ::close(sockfd_); }

void Socket::bindAddress(const InetAddress& addr) {
    sockets::bindOrDie(sockfd_, addr.getSockAddrInet());
}

void Socket::listen() { sockets::listenOrDie(sockfd_); }

int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    // FIXME CHECK
}
void Socket::setTcpNoDelay(bool on)
{
    int optval =on ?1:0;
    ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof(optval));
}
void Socket::setTcpKeepAlive(bool on)
{
    int optval=on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,sizeof(optval));
}
void Socket::shutdownWrite(){
    sockets::shutdownWrite(sockfd_);
}
