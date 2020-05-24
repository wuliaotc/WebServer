//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_HTTPRESPONSE_H
#define WEBSERVER_HTTPRESPONSE_H

#include <string>
#include <map>
#include <base/copyable.h>

namespace reactor {
    namespace net {
        class Buffer;

        class HttpResponse : public reactor::copyable {
        public:
            enum HttpStatusCode {
                kUnknown,
                k200Ok = 200,
                k301MovedPermanently = 301,
                k400BadRequest = 400,
                k404NotFound = 404,
            };

            explicit HttpResponse(bool close)
                    : statusCode_(kUnknown),
                      closeConnection_(close) {
            }

            void setStatusCode(HttpStatusCode code) { statusCode_ = code; }

            void setStatusMessage(
                    const std::string &message) { statusMessage_ = message; }

            void setCloseConnection(bool on) { closeConnection_ = on; }

            bool closeConnection() const { return closeConnection_; }

            void setContentType(const std::string &contentType) {
                addHeader("Content-Type", contentType);
            }

            // FIXME: replace string with StringPiece
            void addHeader(const std::string &key,
                           const std::string &value) { headers_[key] = value; }

            void setBody(const std::string &body) { body_ = body; }

            void appendToBuffer(Buffer *output) const;

        private:
            std::map<std::string, std::string> headers_;
            HttpStatusCode statusCode_;
            // FIXME: add http version
            std::string statusMessage_;
            bool closeConnection_;
            std::string body_;
        };

    }  // namespace net
}  // namespace reactor




#endif //WEBSERVER_HTTPRESPONSE_H
