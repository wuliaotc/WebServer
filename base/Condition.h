//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_CONDITION_H
#define WEBSERVER_CONDITION_H


#include"Mutex.h"
#include<boost/noncopyable.hpp>
#include<errno.h>

namespace reactor {

    class Condition {
    public:
        explicit Condition(MutexLock &mutex) : mutex_(mutex) {
            MCHECK(pthread_cond_init(&cond_, NULL));
        }

        ~Condition() {
            MCHECK(pthread_cond_destroy(&cond_));
        }

        void wait() {
            MutexLock::UnassignGuard ug(mutex_);
            MCHECK(pthread_cond_wait(&cond_, mutex_.getPthreadMutex()));
        }

        void notify() {
            MCHECK(pthread_cond_signal(&cond_));
        }

        void notifyAll() {
            MCHECK(pthread_cond_broadcast(&cond_));
        }

    private:
        MutexLock &mutex_;
        pthread_cond_t cond_;

    };
}


#endif //WEBSERVER_CONDITION_H
