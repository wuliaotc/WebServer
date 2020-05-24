//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_SOCKETSOPS_H
#define WEBSERVER_SOCKETSOPS_H

#include <arpa/inet.h>
#include <endian.h>

namespace reactor {
    namespace net {
        namespace sockets {
//非標準
            inline uint64_t hostToNetwork64(uint64_t host64) {
                return htobe64(host64);
            }

            inline uint32_t hostToNetwork32(uint32_t host32) {
                return htonl(host32);
            }

            inline uint16_t hostToNetwork16(uint16_t host16) {
                return htons(host16);
            }

            inline uint64_t networkToHost64(uint64_t net64) {
                return be64toh(net64);
            }

            inline uint32_t networkToHost32(uint32_t net32) {
                return ntohl(net32);
            }

            inline uint16_t networkToHost16(uint16_t net16) {
                return ntohs(net16);
            }

// create a non-blocking socket file descriptor
// abort if error
            int createNonblockingOrDie();

            int getSocketError(int sockfd);

            void bindOrDie(int sockfd, const struct sockaddr_in &addr);

            void listenOrDie(int sockfd);

            int accept(int sockfd, struct sockaddr_in *addr);

            int connect(int sockfd, const struct sockaddr_in *addr);

            void close(int sockfd);

            void shutdownWrite(int sockfd);

            void
            toHostPort(char *buf, size_t size, const struct sockaddr_in &addr);

            void fromHostPort(const char *ip, uint16_t port,
                              struct sockaddr_in *addr);

            struct sockaddr_in getLocalAddr(int sockfd);

            struct sockaddr_in getPeerAddr(int sockfd);

// sockfd
            bool isSelfConnect(int sockfd);
        } // namespace sockets
    } // namespace net
} // namespace reactor

#endif // WEBSERVER_SOCKETSOPS_H
