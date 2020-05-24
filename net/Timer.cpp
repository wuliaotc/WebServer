//
// Created by andy on 20-5-23.
//

#include "net/Timer.h"
#include <base/Timestamp.h>
//TODO:Timestamp
using namespace reactor;
using namespace reactor::net;
std::atomic_int64_t Timer::s_numCreated_(0);

void Timer::restart(reactor::Timestamp now) {
    if (repeat_) {
        expiration_ = reactor::addTime(now, interval_);
    } else {
        expiration_ = reactor::Timestamp::invalid();
    }
}
