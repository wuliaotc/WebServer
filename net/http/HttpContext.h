//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_HTTPCONTEXT_H
#define WEBSERVER_HTTPCONTEXT_H

#include "base/copyable.h"
#include "net/http/HttpRequest.h"

namespace reactor {

    namespace net {
        class Buffer;

        class HttpContext : public reactor::copyable {
        public:
            enum HttpRequestParseState {
                kExpectRequestLine,
                kExpectHeaders,
                kExpectBody,
                kGotAll,
            };

            HttpContext()
                    : state_(kExpectRequestLine) {}

            bool parseRequest(Buffer *buf, Timestamp receiveTime);

            bool gotAll() const {
                return state_ == kGotAll;
            }

            void reset() {
                state_ = kExpectRequestLine;
                HttpRequest dummy;
                request_.swap(dummy);
            }

            const HttpRequest &request() const {
                return request_;
            }

        private:
            bool processRequestLine(const char *begin, const char *end);


            HttpRequestParseState state_;
            HttpRequest request_;

        };
    }//namespace net
}//namespace reactor

#endif //WEBSERVER_HTTPCONTEXT_H
