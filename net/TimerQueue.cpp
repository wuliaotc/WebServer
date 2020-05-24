//
// Created by andy on 20-5-23.
//

#include "TimerQueue.h"

#include <base/Logging.h>
//TODO:LOG
#include <sys/timerfd.h>

#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

using namespace reactor::net;
namespace reactor {
    namespace detail {
        int createTimerfd() {
            int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                           TFD_CLOEXEC | TFD_NONBLOCK);
            if (timerfd == -1) {
                LOG_SYSFATAL << " Failed in timerfd_create";
            }
            return timerfd;
        }

        struct timespec howMuchTimeFromNow(reactor::Timestamp when) {
            int64_t microseconds = when.microSecondsSinceEpoch() -
                                   reactor::Timestamp::now().microSecondsSinceEpoch();
            if (microseconds < 100) {
                microseconds = 100;
            }
            struct timespec ts;
            ts.tv_sec = static_cast<time_t>(microseconds /
                                            reactor::Timestamp::kMicroSecondsPerSecond);
            ts.tv_nsec = static_cast<time_t>(microseconds %
                                             reactor::Timestamp::kMicroSecondsPerSecond);
            return ts;
        }

        void readTimerfd(int timerfd, reactor::Timestamp now) {
            uint64_t howmany;
            ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
            LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at "
                      << now.toString();
            if (n != sizeof(howmany)) {
                LOG_ERROR << "TimerQueue::handleRead() reads" << n
                          << " instant of 8";
            }
        }

        void resetTimerfd(int timerfd, reactor::Timestamp expiration) {
            // wake up loop by timerfd_settime()
            struct itimerspec newValue;
            struct itimerspec oldValue;
            bzero(&newValue, sizeof newValue);
            bzero(&oldValue, sizeof oldValue);
            newValue.it_value = howMuchTimeFromNow(expiration);
            int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
            if (ret) {
                LOG_SYSERR << "timerfd_settime()";
            }
        }
    }     // namespace detail
}     // namespace reactor
using namespace reactor;
using namespace reactor::detail;
using namespace reactor::net;

TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timerfd_(createTimerfd()),
          timerfdChannel_(loop, timerfd_),
          timers_(),
          callingExpiredTimers_(false) {
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    // update envents and insert to pollfds
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);
    for (TimerList::iterator it = timers_.begin(); it != timers_.end(); ++it) {
        delete it->second;
    }
}

TimerId TimerQueue::addTimerInLoop(Timer *timer) {
    loop_->assertInLoopThread();

    bool earliestChanged = insert(timer);

    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
    return TimerId(timer);
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, reactor::Timestamp when,
                             double interval) {
    Timer *timer = new Timer(cb, when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(reactor::Timestamp now) {
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    TimerList::iterator it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);
    //从事件由早到晚的顺序拷贝,相当于一次插到timers_.end()
    std::copy(timers_.begin(), it, std::back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    for (const Entry &it:expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n = 1);
        (void) n;
    }

    assert(timers_.size() == activeTimers_.size());

    return expired;
}

bool TimerQueue::insert(Timer *timer) {
    bool earliestChanged = false;
    assert(timers_.size() == activeTimers_.size());
    reactor::Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator, bool> result =
                timers_.insert(std::make_pair(when, timer));
        assert(result.second);
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result =
                activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);
    }
    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

void TimerQueue::reset(const std::vector<Entry> &expired,
                       reactor::Timestamp now) {
    reactor::Timestamp nextExpired;
    for (std::vector<Entry>::const_iterator it = expired.begin();
         it != expired.end(); ++it) {
        ActiveTimer timer(it->second, it->second->sequence());
        if (it->second->repeat() &&
            cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            it->second->restart(now);
            insert(it->second);
        } else {
            // FIXME move to a free list
            delete it->second;
        }
    }
    if (!timers_.empty()) {
        nextExpired = timers_.begin()->second->expiration();
    }
    if (nextExpired.valid()) {
        resetTimerfd(timerfd_, nextExpired);
    }
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    reactor::Timestamp now(reactor::Timestamp::now());
    readTimerfd(timerfd_, now);

    callingExpiredTimers_ = true;
    std::vector<Entry> expired = getExpired(now);
    for (auto it = expired.begin(); it != expired.end(); ++it) {
        it->second->run();
    }
    callingExpiredTimers_ = false;
    reset(expired, now);
}

void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);
        (void) n;
        delete it->first;     // FIXME: no delete please
        activeTimers_.erase(it);
    } else if (callingExpiredTimers_) {
        //没找到的原因可能是 该timer已经被取出,且正在执行
        //推迟到reset 进行取消
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}
