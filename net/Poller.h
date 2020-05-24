//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_POLLER_H
#define WEBSERVER_POLLER_H

#include <base/Timestamp.h>

#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

#include "net/EventLoop.h"

struct pollfd;
namespace reactor {
    namespace net {
        class Poller : boost::noncopyable {
        public:
            using ChannelList = std::vector<Channel *>;

            Poller(EventLoop *loop);

            ~Poller();

            void updateChannel(Channel *);

            void removeChannel(Channel *);

            reactor::Timestamp poll(int timeoutMs, ChannelList *activeChannels);

            void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

        private:
            using PollFdList = std::vector<struct pollfd>;
            using ChannelMap = std::map<int, Channel *>;

            EventLoop *ownerLoop_;
            PollFdList pollfds_;
            ChannelMap channels_;

            void fillActiveChannels(int numEvents,
                                    ChannelList *activeChannels) const;
        };
    } // namespace net
} // namespace reactor

#endif // WEBSERVER_POLLER_H
