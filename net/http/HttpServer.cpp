//
// Created by andy on 20-5-23.
//

#include "net/http/HttpServer.h"
#include "net/http/HttpResponse.h"
#include "net/http/HttpContext.h"
#include <base/Logging.h>

using namespace reactor;
using namespace reactor::net;
namespace reactor {
    namespace net {
        void DefaultHttpCallback(const HttpRequest &request,
                                 HttpResponse *response) {
            response->setStatusCode(HttpResponse::k404NotFound);
            response->setStatusMessage("Not Found");
            response->setCloseConnection(true);
        }
    }
}

HttpServer::HttpServer(EventLoop *loop, const InetAddress &listenAddr,
                       int idleSeconds)
        : server_(loop, listenAddr),
          httpCallback_(DefaultHttpCallback),
          idleSecond_(idleSeconds),
          timePointer_(0) {
    server_.setConnectionCallback(
            std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
            std::bind(&HttpServer::onMessage, this, std::placeholders::_1,
                      std::placeholders::_2, std::placeholders::_3));
    assert(idleSeconds >= 0);
    assert(timePointer_ == 0);
    if (idleSeconds > 0) {
        timeWheel_.resize(idleSeconds);
        loop->runEvery(1.0, std::bind(&HttpServer::onTimer, this));
    }
}

void HttpServer::onTimer() {
    TimeWheelBucket bucket;
    bucket.swap(timeWheel_[timePointer_]);
    timePointer_ = (timePointer_ + 1) % idleSecond_;//tick
    int conn_num;

    //DEBUG
//#ifdef DEBUG
    for(auto &&it:bucket){
        bool connectionDead = it->weakConn_.expired();
        LOG_INFO <<"conn at "<<it.get()<<",use_coun="<<it.use_count()
                 <<(connectionDead ? " DEAD" : "");
    }
//#endif
}

void HttpServer::start() {
    //TODO:more info
    LOG_WARN << "HttpServer start!";
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
        conn->setContext(HttpContext());
        if(idleSecond_!=0){
            EntryPtr entry(new Entry(conn));
            timeWheel_[timeWheelEnd()].insert(entry);
            WeakEntryPtr weakEntry(entry);
            conn->setWeakEntryPtr(weakEntry);
        }
    } else {
        if(idleSecond_!=0){
            assert(!conn->getWeakEntryPtr().empty());
            WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getWeakEntryPtr()));
            LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
        }
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn,
                           const HttpRequest &request) {
    const string &connection = request.getHeader("Connection");
    //FIXME:Hack unsupported other methods
    if (request.method() != HttpRequest::kPost &&
        request.method() != HttpRequest::kGet) {
        LOG_WARN << "connection [ " << conn->peerAddress().toIpPort()
                 << " ] use unsported method" << request.methodString();
        conn->send("HTTP/1.1 405 Method not allowed\r\n\r\n");
        conn->shutdown();
        return;
    }
    bool close = connection == "close" ||
                 (request.getVersion() == HttpRequest::kHttp10 &&
                  connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(request, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(buf);
    if (response.closeConnection()) {
        conn->shutdown();
    }
    else if (idleSecond_ != 0) {
        assert(!conn->getWeakEntryPtr().empty());
        WeakEntryPtr weakEntry(
                boost::any_cast<WeakEntryPtr>(conn->getWeakEntryPtr()));
        EntryPtr entry(weakEntry.lock());
        if (entry) {
            timeWheel_[timeWheelEnd()].insert(entry);
//            dumpConnectionBuckets();
        }
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                           Timestamp receiveTime) {
    HttpContext *context = boost::any_cast<HttpContext>(
            conn->getMutableContext());

    if (!context->parseRequest(buf, receiveTime)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
    if (context->gotAll()) {
        onRequest(conn, context->request());
        context->reset();
        //only when we receive a full request ,then we refresh conn timeout
    }


}
//
//void HttpServer::dumpConnectionBuckets() const {
//    LOG_INFO << "size = " << timeWheel_.size();
//    LOG_INFO << "current bucket.size()="<<timeWheel_[timePointer_].size();
//    LOG_INFO << "back bucket.size()="<<timeWheel_[timeWheelEnd()].size();
//    const TimeWheelBucket bucket;
//    timeWheel_[timeWheelEnd()];
////    int idx = 0;
////    for (TimeWheel::const_iterator bucketI = timeWheel_.begin();
////         bucketI != timeWheel_.end();
////         ++bucketI, ++idx) {
////        const TimeWheelBucket &bucket = *bucketI;
////        LOG_INFO << "bucket["<<idx<<" len="<<bucket.size();
////        for (const auto &it : bucket) {
////            bool connectionDead = it->weakConn_.expired();
////            LOG_INFO <<"conn at"<<it.get()<<",use_coun="<<it.use_count()
////                   <<(connectionDead ? " DEAD" : "");
////        }
////        puts("");
////    }
//}