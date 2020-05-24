//
// Created by andy on 20-5-23.
//

#include <net/Channel.h>

#include <base/Logging.h>

#include <poll.h>

#include "net/EventLoop.h"

using namespace reactor;
using namespace reactor::net;
const int Channel::KNoneEvent = 0;
const int Channel::KReadEvent = POLLIN | POLLPRI;
const int Channel::KWirteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
        : loop_(loop),
          fd_(fd),
          events_(Channel::KNoneEvent),
          revents_(Channel::KNoneEvent),
          index_(-1),
          eventHandling_(false) {}

Channel::~Channel() { assert(!eventHandling_); }

void Channel::update() { loop_->updateChannel(this); }

void Channel::handleEvent(reactor::Timestamp receiveTime) {
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

