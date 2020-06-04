//
// Created by andy on 2020/6/4.
//

#include "net/http/HttpUriRouter.h"
#include "net/http/HttpRequest.h"

namespace reactor{
    namespace net{
        namespace detail{
            unsigned char FromHex(unsigned char x)
            {
                unsigned char y;
                if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
                else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
                else if (x >= '0' && x <= '9') y = x - '0';
                else assert(0);
                return y;
            }
            std::string decodeUri(const StringPiece &path){

                std::string strTemp = "";
                size_t length = path.size();
                for (size_t i = 0; i < length; i++) {
                    if (path[i] == '+') strTemp += ' ';
                    else if (path[i] == '%') {
                        // FIXME:hack
                        // change a way to return err?
                        if(i>length-3) return "";//"%"
                        unsigned char high = FromHex((unsigned char) path[++i]);
                        unsigned char low = FromHex((unsigned char) path[++i]);
                        strTemp += high * 16 + low;
                    } else strTemp += path[i];
                }
                return strTemp;

            }
        }
    }
}
using namespace reactor;
using namespace reactor::net;

void HttpUriRouter::registerUri(const std::string &uriRegex, const uriHandlerFunc &handler) {
    //#ifdef DEBUG
    for(auto &&it:uriVec_){
        assert(uriRegex!=it.regexStr_);
    }
//#endif
    uriVec_.emplace_back(uriRegex, handler);
}
void HttpUriRouter::Router(const HttpRequest &req, HttpResponse *resp) {
    std::string url = detail::decodeUri(req.path());
    auto it = uriVec_.begin();
    while (it != uriVec_.end()) {
        if (std::regex_match(url, it->uriRegex_))
            break;
        ++it;
    }
    if (it == uriVec_.end())
        defaultHandler_(req,url,resp);
    else
        it->handler_(req,url,resp);
}