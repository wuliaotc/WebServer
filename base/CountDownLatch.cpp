//
// Created by andy on 20-5-23.
//

#include "CountDownLatch.h"

using namespace reactor;

CountDownLatch::CountDownLatch(int count)
        : mutex_(),
          condition_(mutex_),
          count_(count) {
}

void CountDownLatch::wait() {
    MutexLockGuard lock(mutex_);
    while (count_ > 0) {
        condition_.wait();
    }
}

void CountDownLatch::countDown() {
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0) {
        condition_.notifyAll();
    }
}

int CountDownLatch::getCount() const {
    MutexLockGuard lock(mutex_);
    return count_;
}
// namespace reactor