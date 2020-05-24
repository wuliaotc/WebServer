//
// Created by andy on 20-5-23.
//

#include "net/Acceptor.h"
#include <base/Logging.h>
#include <functional>

#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketsOps.h"

using namespace reactor;
using namespace reactor::net;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenaddr)
        : loop_(loop),
          listenning_(false),
          acceptSocket_(sockets::createNonblockingOrDie()),
          acceptChannel_(loop_, acceptSocket_.fd()) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenaddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->isInLoopThread();
    InetAddress peerAddr(0);
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_)
            newConnectionCallback_(connfd, peerAddr);
        else
            sockets::close(connfd);
    }
}

