//
// Created by andy on 2020/6/4.
//

#ifndef WEBSERVER_HTTPURIROUTER_H
#define WEBSERVER_HTTPURIROUTER_H
/**
        Router("GET",path,handler)


 */

#include "base/noncopyable.h"
#include "base/StringPiece.h"

#include <functional>
#include <regex>
#include <string>
#include <vector>

#include <assert.h>

namespace reactor {
    namespace net {
        namespace detail {
            unsigned char FromHex(unsigned char x);
            std::string decodeUri(const StringPiece &path);
        }
        class HttpRequest;

        class HttpResponse;

        using uriHandlerFunc = std::function<void(const HttpRequest &req,const StringPiece& decodedUrl,
                                                  HttpResponse *resp)>;

        class HttpUriRouter : reactor::noncopyable {
        public:
            HttpUriRouter(const uriHandlerFunc &defalutHandler)
                    : defaultHandler_(defalutHandler) {}

            // don't reg the same uriRegex more than once
            // if not , we will throw std::regex_error
            void registerUri(const std::string &uriRegex,
                             const uriHandlerFunc &handler);

//            bool deleteUri(const std::string& uriRegex){
//                for(auto &&it:uriVec_){
//                    if(uriRegex==it.regexStr_){
//                        it
//                    }
//                }
//            }
            void Router(const HttpRequest &req, HttpResponse *resp);

        private:
//            void encodeUri(){
//
//            }
            struct uriHandler {
                std::string regexStr_;
                std::regex uriRegex_;
                uriHandlerFunc handler_;

                //
                uriHandler(const std::string &uriRegex,
                           const uriHandlerFunc &handler)
                        : regexStr_(uriRegex),
                          uriRegex_(uriRegex),
                          handler_(handler) {}
            };

            std::vector<uriHandler> uriVec_;
            uriHandlerFunc defaultHandler_;
        };
    }
}
#endif //WEBSERVER_HTTPURIROUTER_H
