set(base_SRCS
        Thread.cpp
        CountDownLatch.cpp
        Date.cpp
        Logging.cpp
        LogStream.cpp
        TimeZone.cpp
        Timestamp.cpp
        )

add_library(webserver_base ${base_SRCS})
target_link_libraries(webserver_base pthread)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/webserver/base)

#if(WEBSERVER_BUILD_EXAMPLES)
add_subdirectory(tests)
#endif()
