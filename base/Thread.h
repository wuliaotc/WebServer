//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_THREAD_H
#define WEBSERVER_THREAD_H


#include<pthread.h>
#include<functional>
#include<string>
#include<memory>
#include<atomic>

namespace reactor {

    class Thread {
    public:
        using ThreadFunc = std::function<void()>;

        explicit Thread(const ThreadFunc &,
                        const std::string &name = std::string());

        ~Thread();

        void start();

        void join();

        pid_t tid() const { return *tid_; }

        bool started() const { return started_; }

        static int numCreate() { return numCreate_.load(); }

    private:
        pthread_t PthreadId_;
        std::shared_ptr<pid_t> tid_;
        bool started_;
        bool joined_;
        ThreadFunc func_;
        std::string name_;
        static std::atomic<int32_t> numCreate_;
    };
    namespace CurrentThread {
        //获取线程id,这里使用的缓存优化 避免系统调用
        pid_t tid();

        const char *name();

        //判断是否是主线程
        bool isMainThread();

        const char *tidString();

        size_t tidStringLength();
    };
};


#endif //WEBSERVER_THREAD_H
