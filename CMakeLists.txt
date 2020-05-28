cmake_minimum_required(VERSION 2.6)
project(WebServer C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# only build examples if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL "muduo")
    option(WEBSERVER_BUILD_EXAMPLES "Build test examples" ON)
endif()

set(CXX_FLAGS
        -g
        # -DVALGRIND
        -DCHECK_PTHREAD_RETURN_VALUE
#        -D_FILE_OFFSET_BITS=64
        -Wall
        -Wextra
        -Werror
#        -Wconversion
#        -Wno-unused-parameter
#        -Wold-style-cast
#        -Woverloaded-virtual
#        -Wpointer-arith
#        -Wshadow
#        -Wwrite-strings
#        -march=native
        # -MMD
        #-std=c++11
        #-rdynamic
        )
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR})

find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)

add_subdirectory(base)
add_subdirectory(net)


