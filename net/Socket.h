//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_SOCKET_H
#define WEBSERVER_SOCKET_H

#include <unistd.h>

#include <boost/noncopyable.hpp>

namespace reactor {
    namespace net {
        class InetAddress;

        class Socket : boost::noncopyable {
        public:
            explicit Socket(int sockfd) : sockfd_(sockfd) {}

            ~Socket();

            int fd() const { return sockfd_; }

            void bindAddress(const InetAddress &localaddr);

            void listen();

            int accept(InetAddress *peeraddr);

            void setReuseAddr(bool on);

            void setTcpNoDelay(bool on);

            void setTcpKeepAlive(bool on);

            void shutdownWrite();

        private:
            int sockfd_;
        };
    } // namespace net
} // namespace reactor

#endif // WEBSERVER_SOCKET_H
