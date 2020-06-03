//
// Created by andy on 20-5-23.
//

#include "net/http/HttpContext.h"
#include "net/Buffer.h"
#include "base/Logging.h"
#include <exception>
using namespace reactor;
using namespace reactor::net;
namespace reactor{
    namespace net{
        namespace detail{
            int atoi(const string& str){
                int res=-1;
                try {
                    res=std::stoi(str);
                }
//                catch (std::invalid_argument) {
//                    return -1;
//                }
//                catch (std::out_of_range) {
//                    return -1;
//                }
                catch (...) {
                    //unknow
                    res= -1;
                }
                return res;
            }
        }
    }
}

bool HttpContext::processRequestLine(const char *begin, const char *end) {
    bool succeed = false;
    const char* start=begin;
    /*
    "HTTP/1.0\r\n"
    "Host: 127.0.0.1:8080\r\n"
    "User-Agent: ApacheBench/2.3\r\n"
    //"Accept: * /*\r\n"
    "\r\n"*/
    const char* space=std::find(start,end,' ');
    if(space!=end&&request_.setMethod(start,space))//GET / HTTP1.1\r\n
    {
        start=space+1;
        space=std::find(start,end,' ');// / HTTP1.1\r\n
        if(space!=end)
        {
            const char*question=std::find(start,space,'?');
            if(question!=space)
            {
                request_.setPath(start,question);
                request_.setQuery(question,space);
            }
            else{
                request_.setPath(start,space);
            }
            start=space+1;// / HTTP1.1\r\n
            succeed=(end-start==8)&&std::equal(start,end-1,"HTTP/1.");
            if(succeed)
            {
                if(*(end-1)=='1')
                {
                    request_.setVersion(HttpRequest::kHttp11);
                }
                else if(*(end-1)=='0')
                {
                    request_.setVersion(HttpRequest::kHttp10);
                } else
                {
                    succeed= false;
                }
            }
        }
    }
    return succeed;
}

bool HttpContext::parseRequest (Buffer *buf, Timestamp receiveTime){
    bool ok=true;
    bool hasMore=true;
    while(hasMore)
    {
        if(state_==kExpectRequestLine)
        {
            const char *crlf=buf->findCRLF();
            if (crlf) {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf+2);
                    state_=kExpectHeaders;
                }
                else{
                    hasMore=false;
                }
            }
            else{
                hasMore=false;
            }
        }
        else if(state_==kExpectHeaders)
        {
            const char *crlf=buf->findCRLF();
            if(crlf){
                const char* colon =std::find(buf->peek(),crlf,':');
                if(colon!=crlf){
                    request_.addHeader(buf->peek(),colon,crlf);
                }
                else{
                    //empty line,end of header
                    //FIXME : use low case save header
                    //first read Content-Length
                    assert(request_.getConten_len()==-1);
                    if(request_.hasHeader("Content-Length")) {
                        string content_len_header = request_.getHeader(
                                "Content-Length");
                        if (content_len_header != "") {
                            int len=detail::atoi(content_len_header);
                            if(len>0){
                                request_.setConten_len(len);
                            }
                        }
                    }
                    switch(request_.method()){
                        case HttpRequest::kPost:
                            if(request_.getConten_len()!=-1){
                                state_=kExpectBody;
                                hasMore=true;
                                break;
                            }
                            //then  same as Get
                        case HttpRequest::kGet:
                        default:
                            //FIXME: only support GET andy POST
                            state_=kGotAll;
                            hasMore=false;
                            break;
                    }

                }
                buf->retrieveUntil(crlf+2);
            }
            else{
                hasMore=false;
            }
        }else if(state_==kExpectBody)
        {
            assert(request_.getConten_len()>0);
            if(buf->readableBytes()>=request_.getConten_len()){
                request_.parseContent(buf);
                state_=kGotAll;
            }
            //if timeout the conn will be close
            //so wo need not worry able a very
            //large content_len
            hasMore=false;
        }
    }
    return ok;
}