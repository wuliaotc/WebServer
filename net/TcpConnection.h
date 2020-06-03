//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_TCPCONNECTION_H
#define WEBSERVER_TCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <boost/any.hpp>
#include <memory>
#include <string>

#include "net/Buffer.h"
#include "net/Callbacks.h"
#include "net/InetAddress.h"

namespace reactor {
    namespace net {
        class Channel;

        class Socket;

        class EventLoop;

        class TcpConnection : ::boost::noncopyable,
                              public ::std::enable_shared_from_this<TcpConnection> {
        public:
            TcpConnection(EventLoop *loop, const ::std::string &nameArg,
                          int sockfd,
                          const InetAddress &localAddr,
                          const InetAddress &peerAddr);

            ~TcpConnection();

            void setConnectionCallback(const ConnectionCallback &cb);

            void setMessageCallback(const MessageCallback &cb);

            void setCloseCallback(const CloseCallback &cb);

            void setTcpNoDelay(bool on);

            void setTcpKeepAlive(bool on);

            void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
                writeCompleteCallback_ = cb;
            }

            //	void setHighWaterMarkCallback(const HighWaterMarkCallback &cb){
            //		highWaterMarkCallback_=cb;
            //	}
            bool connected() { return state_ == kConnected; };

            // Thread safe
            void send(const std::string &message);
            void send(Buffer &buf);
            void shutdown();

            // be called when TcpServer accepts a new connection
            void ConnectEstablished(); // should be called only onde
            void ConnectDestroyed();

            const std::string &name() const { return name_; }

            const InetAddress &peerAddress() const { return peerAddr_; }

            const InetAddress &localAddress() const { return localAddr_; }

            EventLoop *getLoop() { return loop_; }

            void setContext(const boost::any& context)
            { context_ = context; }

            const boost::any& getContext() const
            { return context_; }

            boost::any* getMutableContext()
            { return &context_; }
            //only for HttpServer internal use
            void setWeakEntryPtr(const boost::any& weakEntryPtr)
            { weakEntryPtr_ = weakEntryPtr; }
            //only for HttpServer internal use
            const boost::any& getWeakEntryPtr() const
            { return weakEntryPtr_; }
        private:
            enum StateE {
                kConnecting, kConnected, kDisconnecting, kDisconnected
            };

            void setState(StateE s) { state_ = s; }

            void handleRead(reactor::Timestamp receiveTime);

            void handlewrite();

            void handleClose();

            void handleError();

            void sendInLoop(const std::string &message);

            void shutdownInLoop();

            EventLoop *loop_;
            std::string name_;
            StateE state_;
            std::unique_ptr<Socket> socket_;
            std::unique_ptr<Channel> channel_;
            InetAddress localAddr_;
            InetAddress peerAddr_;
            Buffer inputBuffer_;
            Buffer outputBuffer_;
            boost::any context_;
            //FIXME:hack only for HttpServer
            boost::any weakEntryPtr_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            CloseCallback closeCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            //	HighWaterMarkCallback highWaterMarkCallback_;
        };

        using TcpConnectionPtr = ::std::shared_ptr<TcpConnection>;
    } // namespace net
} // namespace reactor

#endif // WEBSERVER_TCPCONNECTION_H
