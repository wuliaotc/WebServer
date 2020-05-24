//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_TIMERID_H
#define WEBSERVER_TIMERID_H

#include<stdint.h>

namespace reactor {
    namespace net {
        class Timer;

///
/// An opaque identifier, for canceling Timer.
///
        class TimerId {
        public:
            TimerId(Timer *timer = nullptr, int64_t seq = 0) : timer_(timer),
                                                               sequence_(seq) {}

            // default copy-ctor, dtor and assignment are okay

            friend class TimerQueue;

        private:
            Timer *timer_;
            int64_t sequence_;
        };
    }//namespace net
}     // namespace reactor



#endif //WEBSERVER_TIMERID_H
