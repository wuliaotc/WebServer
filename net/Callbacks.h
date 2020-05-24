//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_CALLBACKS_H
#define WEBSERVER_CALLBACKS_H


#include <functional>
#include <memory>
#include <unistd.h>
#include <base/Timestamp.h>
//TODO:Timestamp
namespace reactor {
    namespace net {
        using TimerCallback = std::function<void()>;

        class TcpConnection;

        class Buffer;

        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

        using MessageCallback = std::function<void(const TcpConnectionPtr &,
                                                   Buffer *,
                                                   reactor::Timestamp)>;
        using ConnectionCallback = std::function<void(
                const TcpConnectionPtr &)>;
        using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
        using WriteCompleteCallback = std::function<void(
                const TcpConnectionPtr &)>;
        using HighWaterMarkCallback = std::function<void(
                const TcpConnectionPtr &, size_t)>;
    }//namespace net
}


#endif //WEBSERVER_CALLBACKS_H
