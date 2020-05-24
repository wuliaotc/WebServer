//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_COUNTDOWNLATCH_H
#define WEBSERVER_COUNTDOWNLATCH_H

#include "base/Mutex.h"
#include "base/Condition.h"

namespace reactor {

/**
 * @brief 计数器
 * 当指定个数线程准备就绪后才能运行
 */
    class CountDownLatch : boost::noncopyable {
    public:

        explicit CountDownLatch(int count);

        void wait();

        void countDown();

        int getCount() const;

    private:
        mutable MutexLock mutex_;
        Condition condition_ GUARDED_BY(mutex_);
        int count_ GUARDED_BY(mutex_);
    };

} // namespace base



#endif //WEBSERVER_COUNTDOWNLATCH_H
