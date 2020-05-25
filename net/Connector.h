//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_CONNECTOR_H
#define WEBSERVER_CONNECTOR_H

#include <boost/noncopyable.hpp>
#include <functional>

#include "net/Callbacks.h"
#include "net/InetAddress.h"
#include "net/TimerId.h"

namespace reactor {
    namespace net {
        class EventLoop;

        class Channel;

        class Connector : boost::noncopyable {
        public:
            using NewConnectionCallback = std::function<void(int sockfd)>;

            Connector(EventLoop *loop, const InetAddress &serverAddr);

            ~Connector();

            void setNewConnectionCallback(const NewConnectionCallback &cb) {
                newConnectionCallback_ = cb;
            }

            void start();       // can be called in any thread
            void restart();       // must be called in loop thread
            void stop();       // can be called in any thread

            const InetAddress &serverAddress() const { return serverAddr_; }

        private:
            enum States {
                kDisconnected, kConnecting, kConnected
            };
            static const int kMaxRetryDelayMs = 30 * 1000;
            static const int kInitRetryDelayMs = 500;

            void setState(States s) { state_ = s; }

            void startInLoop();

            void connect();

            void connecting(int sockfd);

            void handleWrite();

            void handleError();

            void retry(int sockfd);

            int removeAndResetChannel();

            void resetChannel();

            EventLoop *loop_;
            InetAddress serverAddr_;
            bool connect_;      // atomic
            States state_;      // FIXME: use atomic variable
            std::unique_ptr<Channel> channel_;
            NewConnectionCallback newConnectionCallback_;
            int retryDelayMs_;
            TimerId timerId_;
        };

        using ConnectionPtr = std::shared_ptr<Connector>;
    }//namespace net
}     // namespace reactor


#endif //WEBSERVER_CONNECTOR_H
