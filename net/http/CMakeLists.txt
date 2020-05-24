set(http_SRCS
        HttpServer.cpp
        HttpResponse.cpp
        HttpContext.cpp
        )

add_library(webserver_http ${http_SRCS})
target_link_libraries(webserver_http webserver_base webserver_net)

install(TARGETS webserver_http DESTINATION lib)
set(HEADERS
        HttpContext.h
        HttpRequest.h
        HttpResponse.h
        HttpServer.h
        )
install(FILES ${HEADERS} DESTINATION include/webserver/net/http)

#if(WEBSERVER_BUILD_EXAMPLES)
    add_executable(httpserver_test tests/HttpServerTest.cpp)
    target_link_libraries(httpserver_test webserver_http)
#endif()
#if(WEBSERVER_BUILD_EXAMPLES)
#add_subdirectory(tests)
