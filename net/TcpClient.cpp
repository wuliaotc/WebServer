//
// Created by andy on 20-5-23.
//

#include "net/TcpClient.h"

#include <base/Logging.h>
#include "net/Connector.h"
#include "net/EventLoop.h"
#include "net/SocketsOps.h"

using namespace reactor;
using namespace reactor::net;
namespace reactor {
    namespace net {
        namespace detail {
            void
            removeConnection(EventLoop *loop, const TcpConnectionPtr &conn) {
                loop->queueInLoop(
                        ::std::bind(&TcpConnection::ConnectDestroyed, conn));
            }

            void removeConnector(const ConnectorPtr &connector) {
                // connector->
                //TODO:removeConnector
            }
        }     // namespace detail
    }//namespace net
}     // namespace reactor
TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr)
        : loop_(loop),
          connector_(new Connector(loop, serverAddr)),
          retry_(false),
          connect_(true),
          nextConnId_(1) {
    connector_->setNewConnectionCallback(
            std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
    LOG_INFO << "TcpClient::TcpClient[" << this << "] - connector "
             << connector_.get();
}


TcpClient::~TcpClient() {
    LOG_INFO << "TcpClient::~TcpClient[" << this << "] - connector "
             << connector_.get();
    TcpConnectionPtr conn;
    {
        MutexLockGuard lock(mutex_);
        conn = connection_;
    }
    if (conn) {
        // FIXME: not 100% safe,if we are in different thread
        CloseCallback cb =
                std::bind(&detail::removeConnection, loop_,
                          std::placeholders::_1);
        loop_->runInLoop(
                std::bind(&TcpConnection::setConnectionCallback, conn, cb));
    } else {
        connector_->stop();
        // FIXME: HACK
        loop_->runAfter(1, std::bind(detail::removeConnector, connector_));
    }
}

void TcpClient::connect() {
    // FIXME: check state
    LOG_INFO << "TcpClient::connect[" << this << "] - connecting to "
             << connector_->serverAddress().toHostPort();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;
    {
        MutexLockGuard lock(mutex_);
        if (connector_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toHostPort().c_str(),
             nextConnId_);
    ++nextConnId_;
    std::string connName = buf;

    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    TcpConnectionPtr conn(
            new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
            std::bind(&TcpClient::removeConnection, this,
                      std::placeholders::_1));      // FIXME: unsafe
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->ConnectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
    if (retry_ && connect_) {
        LOG_INFO << "TcpClient::connect[" << this << "] - Reconnecting to "
                 << connector_->serverAddress().toHostPort();
        connector_->restart();
    }
}


