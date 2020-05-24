set(net_SRCS
        Acceptor.cpp
        Buffer.cpp
        Channel.cpp
        Connector.cpp
        EventLoop.cpp
        EventLoopThread.cpp
        EventLoopThreadPool.cpp
        InetAddress.cpp
        Poller.cpp
        Socket.cpp
        SocketsOps.cpp
        TcpClient.cpp
        TcpConnection.cpp
        TcpServer.cpp
        Timer.cpp
        TimerQueue.cpp
        )

add_library(webserver_net ${net_SRCS})
target_link_libraries(webserver_net webserver_base)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/webserver/net)

#if(WEBSERVER_BUILD_EXAMPLES)
add_subdirectory(tests)
#endif()
add_subdirectory(http)
