//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_HTTPSERVER_H
#define WEBSERVER_HTTPSERVER_H
#include <unordered_set>
#include "net/TcpServer.h"

namespace reactor
{
    namespace net
    {

        class HttpRequest;
        class HttpResponse;
/// A simple embeddable HTTP server designed for report status of a program.
/// It is not a fully HTTP 1.1 compliant server, but provides minimum features
/// that can communicate with HttpClient and Web browser.
/// It is synchronous, just like Java Servlet.
        class HttpServer : boost::noncopyable
        {
        public:
            using HttpCallback=std::function<void (const HttpRequest&,
                                        HttpResponse*)> ;
            // if idleSeconds =0 no check for idle connection
            HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,int idleSeconds=0);

            EventLoop* getLoop() const { return server_.getLoop(); }

            /// Not thread safe, callback be registered before calling start().
            void setHttpCallback(const HttpCallback& cb)
            {
                httpCallback_ = cb;
            }

            void setThreadNum(int numThreads)
            {
                server_.setThreadNum(numThreads);
            }

            void start();

        private:
            void onTimer();
            void onConnection(const TcpConnectionPtr& conn);
            void onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime);
            void onRequest(const TcpConnectionPtr&, const HttpRequest&);
            using WeakTcpConnectionPtr=std::weak_ptr<TcpConnection>;

            struct Entry:reactor::copyable{
                explicit Entry(const WeakTcpConnectionPtr& weakConn)
                :weakConn_(weakConn){

                }
                ~Entry(){
                    TcpConnectionPtr conn_=weakConn_.lock();
                    if(conn_){
                        conn_->shutdown();
                    }
                }
                WeakTcpConnectionPtr weakConn_;
            };
            using EntryPtr=std::shared_ptr<Entry>;
            using WeakEntryPtr=std::weak_ptr<Entry>;
            using TimeWheelBucket=std::unordered_set<EntryPtr>;
            using TimeWheel=std::vector<TimeWheelBucket>;
            int timePointer_;
            TimeWheel timeWheel_;
//            //clean
//            void dumpConnectionBuckets()const;
            size_t timeWheelEnd()const{return (timePointer_+idleSecond_-1)%idleSecond_;}
            const int idleSecond_;
            TcpServer server_;
            HttpCallback httpCallback_;
        };

    }  // namespace net
}  // namespace muduo


#endif //WEBSERVER_HTTPSERVER_H
