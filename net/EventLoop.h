//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H


#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <base/Timestamp.h>
#include "base/Thread.h"
#include "net/TimerId.h"
#include "net/Callbacks.h"
#include "base/Condition.h"

namespace reactor {
    namespace net {
        class Channel;

        class Poller;

        class TimerQueue;

        class EventLoop : boost::noncopyable {
        public:
            using Functor = std::function<void()>;

            EventLoop();

            ~EventLoop();

            void loop();

            void quit();

            void assertInLoopThread() {
                if (!isInLoopThread()) {
                    abortNotInLoopThread();
                }
            }

            reactor::Timestamp
            pollReturnTime() const { return pollReturnTime_; }

            TimerId
            runAt(const reactor::Timestamp &time, const TimerCallback &cb);

            TimerId runAfter(double delay, const TimerCallback &cb);

            TimerId runEvery(double interval, const TimerCallback &cb);

            //wakeup EventLoop and call cb immediately
            void runInLoop(const Functor &);

            //queues callback in the loop thread
            //runs after finish poll
            void queueInLoop(const Functor &);

            //internal
            void wakeup();

            void updateChannel(Channel *);

            void removeChannel(Channel *);

            bool isInLoopThread() const {
                return threadId_ == CurrentThread::tid();
            }

            void cancel(TimerId timerId);

        private:
            using ChannelList = std::vector<Channel *>;

            void abortNotInLoopThread();

            void handleRead();//wakeup
            void doPendingFunctors();

            bool looping_; /* atomic */
            bool quit_;
            bool callingPendingFunctors_;/* atomic*/

            const pid_t threadId_;
            reactor::Timestamp pollReturnTime_;

            boost::scoped_ptr<Poller> poller_;
            boost::scoped_ptr<TimerQueue> timerQueue_;
            int wakeupFd_;
            boost::scoped_ptr<Channel> wakeupChannel_;

            reactor::MutexLock mutex_;
            //exposed to other thread ,need protect
            std::vector<Functor> pendingFunctors_
            GUARDED_BY(mutex_);
            ChannelList activeChannels_;

        };
    }//namespace net
}     // namespace reactor




#endif //WEBSERVER_EVENTLOOP_H
