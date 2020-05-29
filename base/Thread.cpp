//
// Created by andy on 20-5-23.
//

#include "Thread.h"
#include <assert.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace reactor {
    namespace CurrentThread {
        __thread const char *t_threadName = "unkonw";
        __thread int t_cachedTid;
        __thread char t_tidString[32];
        __thread int t_tidStringLength;
    }
} // namespace reactor
namespace reactor{
    namespace detail {
        __thread pid_t t_cachedTid = 0;
        pid_t gettid()
        {
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }
        void afterFork() {
            t_cachedTid = detail::gettid();
            reactor::CurrentThread::t_threadName = "main";
        }

        class ThreadNameInitializer {
        public:
            ThreadNameInitializer() {
                reactor::CurrentThread::t_threadName = "main";
                pthread_atfork(NULL, NULL, &afterFork);
            }
        };

        ThreadNameInitializer init;

        struct ThreadData {
            //TODO: ThreadDdata类的作用
            using ThreadFunc = reactor::Thread::ThreadFunc;
            ThreadFunc func_;
            std::string name_;
            std::weak_ptr<pid_t> wkTid;

            ThreadData(const ThreadFunc &func, const std::string &name,
                       const std::shared_ptr<pid_t> &tid)
                    : func_(func), name_(name), wkTid(tid) {}


            void runInThread() {
                pid_t tid = reactor::CurrentThread::tid();
                std::shared_ptr<pid_t> ptid = wkTid.lock();
                if (ptid) {
                    *ptid = tid;
                    ptid.reset();
                }
                reactor::CurrentThread::t_threadName =
                        name_.empty() ? "muduoThread" : name_.c_str();
                ::prctl(PR_SET_NAME, reactor::CurrentThread::t_threadName);
                func_();
                reactor::CurrentThread::t_threadName = "finished";
            }


        };

        void *startThread(void *obj) {
            ThreadData *data = static_cast<ThreadData *>(obj);
            data->runInThread();
            delete data;
            return NULL;
        }
    }

} // namespace
using namespace reactor;

pid_t CurrentThread::tid() {
    if (t_cachedTid == 0) {
        t_cachedTid = reactor::detail::gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d",
                                     t_cachedTid);
    }
    return t_cachedTid;
}

const char *CurrentThread::name() {
    return t_threadName;
}

bool CurrentThread::isMainThread() {
    return tid() == ::getpid();
}

const char *CurrentThread::tidString() {
    return t_tidString;
}

size_t CurrentThread::tidStringLength() {
    return t_tidStringLength;
}

std::atomic<int32_t> Thread::numCreate_;

Thread::Thread(const ThreadFunc &func, const std::string &name)
        : started_(false), joined_(false), tid_(new pid_t(0)), PthreadId_(0),
          func_(func), name_(name) {
    Thread::numCreate_.fetch_add(1);
}


Thread::~Thread() {
    if (started_ && !joined_) {
        pthread_detach(PthreadId_);
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;

    detail::ThreadData *data = new detail::ThreadData(func_, name_, tid_);
    if (pthread_create(&PthreadId_, NULL, &detail::startThread, data) != 0) {
        started_ = false;
        delete data;
        abort();
    }
}

void Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    pthread_join(PthreadId_, NULL);
}
