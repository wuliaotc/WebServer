//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_TCPCLIENT_H
#define WEBSERVER_TCPCLIENT_H

#pragma once

#include "base/Mutex.h"
#include "net/TcpConnection.h"
#include <boost/noncopyable.hpp>
#include <memory>

namespace reactor {
    namespace net {
        class Connector;

        using ConnectorPtr = std::shared_ptr<Connector>;

        class TcpClient : boost::noncopyable {
        public:
            TcpClient(EventLoop *loop, const InetAddress &serverAddr);

            ~TcpClient();

            void connect();

            void disconnect();

            void stop();

            TcpConnectionPtr connection() const {
                MutexLockGuard lock(mutex_);
                return connection_;
            }

            bool retry() const{return retry_;}

            void enableRetry() { retry_ = true; }

            // not thread safe
            void setConnectionCallback(const ConnectionCallback &cb) {
                connectionCallback_ = cb;
            }

            // not thread safe
            void setMessageCallback(
                    const MessageCallback &cb) { messageCallback_ = cb; }

            void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
                writeCompleteCallback_ = cb;
            }

        private:
            void newConnection(int sockfd);

            void removeConnection(const TcpConnectionPtr &conn);

            EventLoop *loop_;
            ConnectorPtr connector_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;

            bool retry_;   // atomic
            bool connect_; // atomic

            int nextConnId_;
            mutable MutexLock mutex_;
            TcpConnectionPtr connection_ GUARDED_BY(mutex_);
        };
    } // namespace net
} // namespace reactor

#endif // WEBSERVER_TCPCLIENT_H
