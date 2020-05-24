//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_TIMERQUEUE_H
#define WEBSERVER_TIMERQUEUE_H


#include <base/Timestamp.h>
//TODO:Timestamp
#include <map>
#include <set>
#include <vector>

#include "Callbacks.h"
#include "Channel.h"

namespace reactor {
    namespace net {
        class EventLoop;

        class TimerId;

        class Timer;

        class TimerQueue {
        public:
            TimerQueue(EventLoop *);

            ~TimerQueue();

            TimerId addTimer(const TimerCallback &cb, reactor::Timestamp when,
                             double interval);

            // void cancel(TimerId timerId);
            TimerId addTimerInLoop(Timer *timer);

            void cancel(TimerId timerId);

        private:
            using Entry = std::pair<reactor::Timestamp, Timer *>;
            using TimerList = std::set<Entry>;
            using ActiveTimer = std::pair<Timer *, int64_t>;
            using ActiveTimerSet = std::set<ActiveTimer>;

            void handleRead();

            void cancelInLoop(TimerId timerId);

            std::vector<Entry> getExpired(reactor::Timestamp now);

            void
            reset(const std::vector<Entry> &expired, reactor::Timestamp now);

            bool insert(Timer *);

            EventLoop *loop_;
            const int timerfd_;
            Channel timerfdChannel_;
            //Timer list sorted by expiration
            TimerList timers_;

            //for cancel
            bool callingExpiredTimers_;/* atomic */
            ActiveTimerSet activeTimers_;
            ActiveTimerSet cancelingTimers_;
        };
    }//namespace net
}     // namespace reactor



#endif //WEBSERVER_TIMERQUEUE_H
