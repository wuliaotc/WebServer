//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_EVENTLOOPTHREADPOOL_H
#define WEBSERVER_EVENTLOOPTHREADPOOL_H

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "net/EventLoopThread.h"

namespace reactor {
    namespace net {
        class EventLoopThreadPool : boost::noncopyable {
        public:
            EventLoopThreadPool(EventLoop *baseLoop);

            ~EventLoopThreadPool();

            void setThreadNum(int numThreads) { numThreads_ = numThreads; }

            void start();

            EventLoop *getNextLoop();

        private:
            EventLoop *baseLoop_;
            bool started_;
            int numThreads_;
            int next_;
            boost::ptr_vector<EventLoopThread> threads_;
            std::vector<EventLoop *> loops_;
        };
    }//namesapce net
}     // namespace reactor




#endif //WEBSERVER_EVENTLOOPTHREADPOOL_H
