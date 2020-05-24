//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_TCPSERVER_H
#define WEBSERVER_TCPSERVER_H


#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>

#include "Callbacks.h"
#include "net/TcpConnection.h"
#include "net/EventLoopThreadPool.h"

namespace reactor {
    namespace net {
        class Acceptor;

        class EventLoop;

        class TcpServer {
        public:
            TcpServer(EventLoop *loop, const InetAddress &listenAddr);

            ~TcpServer();

            // thread safe
            void start();

            // no thread safe
            void setMessageCallback(const MessageCallback &cb) {
                messageCallback_ = cb;
            }

            // no thread safe
            void setConnectionCallback(const ConnectionCallback &cb) {
                connectionCallback_ = cb;
            }

            void setWriteCallback(const WriteCompleteCallback &cb) {
                writeCompleteCallback_ = cb;
            }

            /// set the number of threads for handling input
            //
            // always accepts new connection in loop's thread
            // Must be called before start()
            // @param numThread
            // 0 means all I/O in loop's thread,no thread will created.
            // 1 means all I/O in another thread
            // N means a thread poll with Nthread, new connections are
            // assigned on a round-robin basis
            void setThreadNum(int numThreads);
            //Thread safe
            EventLoop * getLoop()const{return loop_;}
        private:
            void newConnection(int sockfd, const InetAddress &peerAddr);

            //not thread safe ,but inLoop
            void removeConnection(const TcpConnectionPtr &connf);

            void removeConnectionInLoop(const TcpConnectionPtr &conn);

            using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
            ConnectionMap connections_;

            MessageCallback messageCallback_;
            ConnectionCallback connectionCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            EventLoop *loop_;
            const std::string name_;
            boost::scoped_ptr<Acceptor> acceptor_;
            boost::scoped_ptr<EventLoopThreadPool> threadPool_;
            bool started;
            int nextConnId_;
        };
    }//namespace net
}     // namespace reactor



#endif //WEBSERVER_TCPSERVER_H
