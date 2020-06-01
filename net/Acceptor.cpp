//
// Created by andy on 20-5-23.
//

#include "net/Acceptor.h"
#include <base/Logging.h>
#include <functional>

#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketsOps.h"

#include <fcntl.h>
using namespace reactor;
using namespace reactor::net;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenaddr)
    : loop_(loop), listenning_(false),
      acceptSocket_(sockets::createNonblockingOrDie()),
      acceptChannel_(loop_, acceptSocket_.fd()),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(idleFd_ >= 0);
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.bindAddress(listenaddr);
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}
Acceptor::~Acceptor() {
  ::close(idleFd_);
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
  } else {
    LOG_SYSERR << "in Acceptor::handleRead accept failed";
    /// when no more available fd to accept connf,
    // we can't close conn because we can't accept them,
    // this event will always wake up EventLoop until
    // we hava available fd to accept the conn
    // this situation will not settle in
    // a short period of time
    // So we prepare a idle fd to accept the conn
    // then close it
    if (errno == EMFILE) {
      // All file descriptors available to the process are currently open
      // man accept
      ::close(idleFd_); // get a available fd
      // then accept a connection
      idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
      // close th connection
      ::close(idleFd_);
      ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}
