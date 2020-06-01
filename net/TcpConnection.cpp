//
// Created by andy on 20-5-23.
//
#include "net/TcpConnection.h"

#include <base/Logging.h>
#include <errno.h>
#include <stdio.h>

#include <functional>

#include "net/Callbacks.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "net/Socket.h"
#include "net/SocketsOps.h"

using namespace reactor;
using namespace reactor::net;

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name,
                             int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
        : loop_(loop), name_(name), state_(kConnecting), localAddr_(localAddr),
          peerAddr_(peerAddr), socket_(new Socket(sockfd)),
          channel_(new Channel(loop, sockfd)) {
    channel_->setReadCallback(
            std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handlewrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
              << " fd=" << sockfd;
    socket_->setTcpKeepAlive(true);
}

TcpConnection::~TcpConnection() {}

void TcpConnection::setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
}

void TcpConnection::setMessageCallback(const MessageCallback &cb) {
    messageCallback_ = cb;
}

void TcpConnection::setCloseCallback(const CloseCallback &cb) {
    closeCallback_ = cb;
}

void TcpConnection::setTcpNoDelay(bool on) { socket_->setTcpNoDelay(on); }

void TcpConnection::setTcpKeepAlive(bool on) { socket_->setTcpKeepAlive(on); }

void TcpConnection::handleRead(reactor::Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    else if (n == 0)
        handleClose();
    else
        handleError();
}

void TcpConnection::handlewrite() {
    //这里不需要处理错误的理由是错误在handleRead中处理完了
    //这里依赖于Channel的handleEvent的顺序
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(),
                            outputBuffer_.readableBytes());
        //发送的数据是outputBuffer里面的可读数据
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                // writerIndex==readIndex
                channel_->disableWriting();
                if (writeCompleteCallback_)
                    loop_->queueInLoop(
                            std::bind(writeCompleteCallback_,
                                      shared_from_this()));
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            } else {
                LOG_TRACE << "I am going to write more data";
            }
        } else {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    } else {
        LOG_TRACE << "Connection is down,no more writing";
    }
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_
              << "] - SO_ERROR = " << err << " " << reactor::strerror_tl(err);
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state_;
    assert(state_ == kConnected || state_ == kDisconnecting);
    channel_->disableAll();
    // must be the last line
    closeCallback_(shared_from_this());
}

void TcpConnection::ConnectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::ConnectDestroyed() {
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    loop_->removeChannel(channel_.get());
}

void TcpConnection::shutdown() {
    // FIXME: use compare and swap
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::send(const std::string &message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            loop_->runInLoop(
                    std::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

// FIXME efficiency
void TcpConnection::send(Buffer &buf) {
    if (state_ == kConnected) {
        string message(buf.peek(), buf.readableBytes());
        if (loop_->isInLoopThread()) {

            sendInLoop(message);
        } else {
            loop_->runInLoop(
                    std::bind(&TcpConnection::sendInLoop, this, message));
        }
        buf.retrieveAll();
    }
}

// FIXME efficiency
void TcpConnection::sendInLoop(const std::string &message) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if (nwrote >= 0) {
            if (static_cast<size_t>(nwrote) < message.size()) {
                LOG_TRACE << "I am going ti write more data";
            } else if (writeCompleteCallback_) {
                loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_SYSERR << "TcpConnection::sendInLoop";
            }
        }
    }
    assert(nwrote >= 0);
    if (static_cast<size_t>(nwrote) < message.size()) {
        outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}
