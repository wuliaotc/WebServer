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

HttpServer::HttpServer(EventLoop *loop, const InetAddress &listenAddr)
        : server_(loop, listenAddr),
          httpCallback_(DefaultHttpCallback) {
    server_.setConnectionCallback(
            std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
            std::bind(&HttpServer::onMessage,this , std::placeholders::_1,
                      std::placeholders::_2,std::placeholders::_3));
}

void HttpServer::start() {
    //TODO:more info
    LOG_WARN << "HttpServer start!";
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
        conn->setContext(HttpContext());
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn,
                           const HttpRequest &request) {
    const string &connection = request.getHeader("Connection");
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
    }
}
