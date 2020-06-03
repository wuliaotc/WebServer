//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_HTTPREQUEST_H
#define WEBSERVER_HTTPREQUEST_H

#include <map>
#include "net/Buffer.h"
#include "base/copyable.h"
#include "base/Timestamp.h"
#include "base/Types.h"
#include <assert.h>
namespace reactor {
    namespace net {
        class HttpRequest : public reactor::copyable {
        public:
            enum Method {
                kInvalid,
                kGet,
                kPost,
                kHead,
                kPut,
                kDelete
            };
            enum Version {
                kUnknown,
                kHttp10,
                kHttp11
            };

            HttpRequest()
                    : method_(kInvalid),
                      version_(kUnknown),
                      content_len_(-1){}

            void setVersion(Version v) {
                version_ = v;
            }

            Version getVersion() const {
                return version_;
            }

            bool setMethod(const char *start, const char *end) {
                assert(method_ == kInvalid);
                string m(start, end);
                if (m == "GET") {
                    method_ = kGet;
                } else if (m == "POST") {
                    method_ = kPost;
                } else if (m == "HEAD") {
                    method_ = kHead;
                } else if (m == "PUT") {
                    method_ = kPut;
                } else if (m == "DELETE") {
                    method_ = kDelete;
                } else {
                    method_ = kInvalid;
                }
                return method_ != kInvalid;
            }
            Method method() const {
                return method_;
            }

            const char *methodString() const {
                const char *result = "UNKNOWN";
                switch (method_) {
                    case kGet:
                        result = "GET";
                        break;
                    case kPost:
                        result = "POST";
                        break;
                    case kHead:
                        result = "HEAD";
                        break;
                    case kPut:
                        result = "PUT";
                        break;
                    case kDelete:
                        result = "DELETE";
                        break;
                    default:
                        break;
                }
                return result;
            }

            void setPath(const char *start, const char *end) {
                path_.assign(start, end);
            }

            const string &path() const {
                return path_;
            }

            void setQuery(const char *start, const char *end) {
                query_.assign(start, end);
            }

            void setReceiveTime(Timestamp t) {
                receiveTime_ = t;
            }

            void
            addHeader(const char *start, const char *colon, const char *end) {
                string field(start, colon);
                ++colon;
                while (colon < end && isspace(*colon)) {
                    ++colon;
                }
                string value(colon, end);
                while (!value.empty() && isspace(value[value.size() - 1])) {
                    value.resize(value.size() - 1);
                }
                headers_[field] = value;


            }

            string getHeader(const string &field) const {
                string result;
                std::map<string, string>::const_iterator it = headers_.find(
                        field);
                if (it != headers_.end()) {
                    result = it->second;
                }
                return result;
            }
            bool hasHeader(const string&field){
                return headers_.find(field)!=headers_.end();
            }
            const std::map<string, string> &headers() const {
                return headers_;
            }

            void swap(HttpRequest &rhs) {
                std::swap(version_, rhs.version_);
                std::swap(method_, rhs.method_);
                std::swap(path_, rhs.path_);
                std::swap(query_, rhs.query_);
                std::swap(receiveTime_, rhs.receiveTime_);
                std::swap(headers_, rhs.headers_);
            }
            int getConten_len ()const{
                return content_len_;
            }
            // assert(len>0)
            void setConten_len (int len){
                assert(len>0);
                content_len_ = len;
            }
            //should be call only once
            void parseContent(net::Buffer *buf){
                assert(content_len_>0);
                assert(content_.readableBytes()==0);
                assert(buf->readableBytes()>=content_len_);
                content_.append(buf->peek(),content_len_);
                buf->retrieve(content_len_);
                assert(content_.readableBytes()==content_len_);
            }

        private:
            Version version_;
            Method method_;
            //FIXME:int will limit request content max len
            int content_len_;
            Buffer content_;
            string path_;
            string query_;
            Timestamp receiveTime_;
            std::map<string, string> headers_;
        };
    }
}


#endif //WEBSERVER_HTTPREQUEST_H
