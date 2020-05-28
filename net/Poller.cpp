//
// Created by andy on 20-5-23.
//

#include <map>
#include <vector>
#include <algorithm>
#include <poll.h>
#include <assert.h>

#include "net/Channel.h"
#include "net/Poller.h"
#include <base/Logging.h>

// TODO:LOG
struct pollfd;
using namespace reactor;
using namespace reactor::net;

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {}

Poller::~Poller() {}

reactor::Timestamp Poller::poll(int timeoutMs, ChannelList *activeChannels) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    reactor::Timestamp now(reactor::Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        LOG_TRACE << " nothing happended";
    } else {
        LOG_SYSERR << "Poller::poll";
    }
    return now;
}

void Poller::updateChannel(Channel *channel) {
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events";
    if (channel->index() < 0) {
        // a new channel
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = channel->events();
        pfd.revents = 0;

        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;

        channel->set_index(idx);
        channels_[pfd.fd] = channel; // insert map
    } else {                       // update one has exist
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);

        int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        //-1代表Channel暂时不关心事件
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {

            pfd.fd = -channel->fd() - 1; //忽略事件
            /*改进做法
              pfd.fd=-(channel->fd())-1
              //因为fd是从0开始的 所以要减去1
              //用channel的fd的负数减去1来代表暂时不关心该事件
              //但是未来可能重新被激活
            */
        }
    }
}

void Poller::removeChannel(Channel *channel) {
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd &pfd = pollfds_[idx];
    (void) pfd;
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1);
    (void) n;
    if (reactor::implicit_cast<size_t>(idx) == pollfds_.size() - 1) {
        pollfds_.pop_back();
    } else {
        int channelAtEnd = pollfds_.back().fd;
        std::iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd -1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}

void Poller::fillActiveChannels(int numEvents,
                                ChannelList *activeChannels) const {
    for (PollFdList::const_iterator pfd = pollfds_.begin();
         pfd != pollfds_.end() && numEvents > 0; ++pfd) {

        if (pfd->revents > 0) {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel *channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}