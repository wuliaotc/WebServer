#include "net/EventLoopThreadPool.h"
#include<boost/ptr_container/ptr_vector.hpp>

using namespace reactor;
using namespace reactor::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop)
        : baseLoop_(baseLoop),
          started_(false),
          numThreads_(0),
          next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
//dont delete loop,because loop is a stack var
}

void EventLoopThreadPool::start() {
    assert(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        EventLoopThread *t = new EventLoopThread;
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    EventLoop *loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    //empty return baseLoop
    return loop;
}
