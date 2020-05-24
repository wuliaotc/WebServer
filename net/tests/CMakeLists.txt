add_executable(channel_test Channel_test.cpp)
target_link_libraries(channel_test webserver_net)

if (BOOSTTEST_LIBRARY)

endif ()