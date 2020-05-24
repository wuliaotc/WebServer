//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H

#pragma  once

#include <boost/noncopyable.hpp>

#include "Callbacks.h"
#include <base/Timestamp.h>
#include <atomic>

namespace reactor {

    namespace net {
///
/// Internal class for timer event.
///
        class Timer : boost::noncopyable {
        public:
            Timer(const TimerCallback &cb, reactor::Timestamp when,
                  double interval)
                    : callback_(cb), expiration_(when), interval_(interval),
                      repeat_(interval > 0.0),
                      sequence_(s_numCreated_.fetch_add(1)) {}

            void run() const { callback_(); }

            reactor::Timestamp expiration() const { return expiration_; }

            bool repeat() const { return repeat_; }

            int64_t sequence() { return sequence_; }

            void restart(reactor::Timestamp now);

        private:
            const TimerCallback callback_;
            reactor::Timestamp expiration_;
            const double interval_;
            const bool repeat_;
            const int64_t sequence_;

            static std::atomic_int64_t s_numCreated_;
        };
    }//namesapce net
} // namespace reactor

#endif //WEBSERVER_TIMER_H
