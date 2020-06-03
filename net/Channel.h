//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_CHANNEL_H
#define WEBSERVER_CHANNEL_H
#pragma once

#include "base/Timestamp.h"
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>

namespace reactor {
    namespace net {
        class EventLoop;

        class Channel : boost::noncopyable {
        public:
            using EventCallback = std::function<void()>;
            using ReadEventCallback = std::function<void(Timestamp)>;

            Channel(EventLoop *loop, int fd);

            ~Channel();

            void handleEvent(Timestamp receiveTime);

            void
            setReadCallback(const ReadEventCallback &cb) { readCallback_ = cb; }

            void
            setWriteCallback(const EventCallback &cb) { writeCallback_ = cb; }

            void
            setErrorCallback(const EventCallback &cb) { errorCallback_ = cb; }

            void
            setCloseCallback(const EventCallback &cb) { closeCallback_ = cb; };

            bool isNoneEvent() { return events_ == kNoneEvent; }

            void enableReading() {
                events_ |= kReadEvent;
                update();
            }

            void enableWriting() {
                events_ |= kWirteEvent;
                update();
            }

            void disableWriting() {
                events_ &= ~kWirteEvent;
                update();
            }

            void disableAll() {
                events_ = kNoneEvent;
                update();
            }

            bool isWriting() const { return events_ & kWirteEvent; }

            bool isReading() const { return events_ & kReadEvent; }

            //sahred_ptr v
            void tie(const std::shared_ptr<void> &obj) {
                tie_ = obj;
                tied_ = true;
            }
            void remove();
            int fd() const { return fd_; }

            int events() const { return events_; }

            void set_revents(int revt) { revents_ = revt; }

            /// for Poller
            int index() const { return index_; };

            void set_index(int idx) { index_ = idx; };

            EventLoop *ownerLoop() { return loop_; };

        private:
            static const int kNoneEvent;
            static const int kReadEvent;
            static const int kWirteEvent;

            void update();

            void handleEventWithGuard(Timestamp receiveTime);

            EventLoop *loop_;
            const int fd_;
            int events_;
            int revents_;
            int index_; // used by Poller

            bool addedToLoop_;
            //避免Channel的handEvent调用前
            //Channel的owner已经调用remove channel
            //导致channel被析构
            bool tied_;
            std::weak_ptr<void> tie_;

            bool eventHandling_;
            ReadEventCallback readCallback_;
            EventCallback writeCallback_;
            EventCallback errorCallback_;
            EventCallback closeCallback_;
        };
    } // namespace net
} // namespace reactor

#endif // WEBSERVER_CHANNEL_H
