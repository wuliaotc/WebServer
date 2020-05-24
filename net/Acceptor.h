//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_ACCEPTOR_H
#define WEBSERVER_ACCEPTOR_H


#include <boost/noncopyable.hpp>
#include <functional>

#include "net/Channel.h"
#include "net/Socket.h"

namespace reactor {
    namespace net {
        class EventLoop;

        class InetAddress;

        class Acceptor : boost::noncopyable {
        public:
            using NewConnectionCallback = std::function<void(int sockfd,
                                                             const InetAddress &)>;

            Acceptor(EventLoop *loop, const InetAddress &ListenAddr);

            void setNewConnectionCallback(const NewConnectionCallback &cb) {
                newConnectionCallback_ = cb;
            }

            bool listenning() const { return listenning_; }

            void listen();

        private:
            void handleRead();

            EventLoop *loop_;
            Socket acceptSocket_;
            Channel acceptChannel_;
            NewConnectionCallback newConnectionCallback_;
            bool listenning_;
        };
    }//namespace net
}//namespace reactor



#endif //WEBSERVER_ACCEPTOR_H
