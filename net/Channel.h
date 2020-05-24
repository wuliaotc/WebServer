//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_CHANNEL_H
#define WEBSERVER_CHANNEL_H
#pragma once

#include <boost/noncopyable.hpp>
#include <functional>
#include "base/Timestamp.h"

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

            bool isNoneEvent() { return events_ == KNoneEvent; }

            int events() const { return events_; }

            void set_revents(int revt) { revents_ = revt; }

            void enableReading() {
                events_ |= KReadEvent;
                update();
            }

            void enableWriting() {
                events_ |= KWirteEvent;
                update();
            }

            void disableWriting() {
                events_ &= ~KWirteEvent;
                update();
            }

            void disableAll() {
                events_ = KNoneEvent;
                update();
            }

            bool isWriting() const {
                return events_ & KWirteEvent;
            }

            int fd() const { return fd_; }

            /// for Poller
            int index() const { return index_; };

            void set_index(int idx) { index_ = idx; };

            EventLoop *ownerLoop() { return loop_; };
        private:
            static const int KNoneEvent;
            static const int KReadEvent;
            static const int KWirteEvent;

            void update();

            const int fd_;

            bool eventHandling_;
            int events_;
            int revents_;
            EventLoop *loop_;
            int index_;
            ReadEventCallback readCallback_;
            EventCallback writeCallback_;
            EventCallback errorCallback_;
            EventCallback closeCallback_;
        };
    }//namespace net
}     // namespace reactor


#endif //WEBSERVER_CHANNEL_H
