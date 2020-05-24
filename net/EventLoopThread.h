//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_EVENTLOOPTHREAD_H
#define WEBSERVER_EVENTLOOPTHREAD_H


#include "net/EventLoop.h"

namespace reactor {
    namespace net {
        class EventLoopThread {
        public:
            EventLoopThread();

            ~EventLoopThread();

            EventLoop *startLoop();

        private:
            void threadFunc();

            EventLoop *loop_;
            Thread thread_;
            bool exiting_;

            MutexLock mutex_;
            Condition cond_;
        };
    }//namespace net
}     // namespace reactor



#endif //WEBSERVER_EVENTLOOPTHREAD_H
