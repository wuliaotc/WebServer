//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_INETADDRESS_H
#define WEBSERVER_INETADDRESS_H

#include <netinet/in.h>

#include <string>

namespace reactor {
    namespace net {
        class InetAddress {
            //儲存 sockaddr_in
        public:
            explicit InetAddress(uint16_t port);

            InetAddress(const std::string &ip, uint16_t port);

            InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

            InetAddress(const InetAddress &) = default;

            InetAddress &operator=(const InetAddress &) = default;

            std::string toHostPort() const;

            const struct sockaddr_in &getSockAddrInet() const { return addr_; }

            void
            setSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; }

        private:
            sockaddr_in addr_;
        };
    } // namespace net
} // namespace reactor

#endif // WEBSERVER_INETADDRESS_H
