//
// Created by andy on 20-5-23.
//

#include <net/Channel.h>

#include <base/Logging.h>

#include <poll.h>

#include "net/EventLoop.h"

using namespace reactor;
using namespace reactor::net;
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWirteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
        : tied_(false),
          loop_(loop),
          fd_(fd),
          events_(Channel::kNoneEvent),
          revents_(Channel::kNoneEvent),
          index_(-1),
          eventHandling_(false),
          addedToLoop_(false){}

Channel::~Channel() {
    //TODO: fixme
    // FIXME will abort()!!
    assert(!eventHandling_);
    assert(!addedToLoop_);
}

void Channel::update() {
    addedToLoop_=true;
    loop_->updateChannel(this);
}
void Channel::handleEvent(Timestamp receiveTime) {
  if(tied_){
    std::shared_ptr<void> guard=tie_.lock();
    if(guard){
      handleEventWithGuard(receiveTime);
    }
  }
  else{
    handleEventWithGuard(receiveTime);
  }
}
void Channel::handleEventWithGuard(reactor::Timestamp receiveTime) {
    eventHandling_ = true;
    if (revents_ & POLLNVAL) {      // fd invalid
        LOG_WARN << "Channel::handle_event() POLLNVAL";
    }
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {     //连接断开且无数据可读
        LOG_WARN << "Channel::handle_event() POLLHUP";
        if (closeCallback_) closeCallback_();
    }
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) readCallback_(receiveTime);
    }
    if (revents_ & POLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    eventHandling_ = false;
}
void Channel::remove() {
    assert(isNoneEvent());
    addedToLoop_=false;
    loop_->removeChannel(this);
}

