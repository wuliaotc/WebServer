//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_INETADDRESS_H
#define WEBSERVER_INETADDRESS_H

#include <netinet/in.h>

#include <string>
#include <base/copyable.h>
#include <base/StringPiece.h>
#include "net/SocketsOps.h"

namespace reactor {
    namespace net {
        class InetAddress : public copyable {
            //儲存 sockaddr_in
        public:
            explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false,
                                 bool ipv6 = false);

            InetAddress(const StringArg &ip, uint16_t port, bool ipv6 = false);

            explicit InetAddress(const struct sockaddr_in &addr) : addr_(
                    addr) {}

            explicit InetAddress(const struct sockaddr_in6 &addr) : addr6_(
                    addr) {}

            sa_family_t family() const { return addr_.sin_family; }

            std::string toIpPort() const;

            const struct sockaddr *getSockAddrInet() const {
                return static_cast<const sockaddr *>(implicit_cast<const void *>(
                        &addr6_));
            }


            void
            setSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; }
            void
            setSockAddrInet6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }
        private:
            union {
                sockaddr_in addr_;
                sockaddr_in6 addr6_;
            };

        };
    } // namespace net
} // namespace reactor

#endif // WEBSERVER_INETADDRESS_H
