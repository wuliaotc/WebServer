add_executable(channel_test Channel_test.cpp)
target_link_libraries(channel_test webserver_net)

if(BOOSTTEST_LIBRARY)
    add_executable(buffer_unit_test Buffer_unittest.cpp)
    target_link_libraries(buffer_unit_test webserver_net boost_unit_test_framework)
    add_test(NAME buffer_unit_test COMMAND buffer_unit_test)
endif()