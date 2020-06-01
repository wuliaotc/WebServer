//
// Created by andy on 20-5-23.
//

#include "TcpServer.h"

#include <base/Logging.h>
//TODO:LOG
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"

using namespace reactor;
using namespace reactor::net;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr)
        : loop_(loop),
          name_(listenAddr.toIpPort()),
          started(false),
          nextConnId_(1),
          threadPool_(new EventLoopThreadPool(loop)),
          acceptor_(new Acceptor(loop, listenAddr)) {
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,
                                                  this, std::placeholders::_1,
                                                  std::placeholders::_2));
}

TcpServer::~TcpServer() {}

void TcpServer::start() {
    if (!started) {
        started = true;
        threadPool_->start();
    }
    if (!acceptor_->listenning()) {
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(numThreads >= 0);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection ["
             << connName << "] from " << peerAddr.toIpPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME: poll with zero timeout to douvle confirm the new connection'
    EventLoop *ioLoop = threadPool_->getNextLoop();

    TcpConnectionPtr conn(
            new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    // FIXME: unsafe
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, conn));
    ioLoop->runInLoop(std::bind(&TcpConnection::ConnectEstablished, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnection [" << name_ << "] - connection "
             << conn->name();
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    (void) n;
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

