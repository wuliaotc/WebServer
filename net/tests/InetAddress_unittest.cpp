//
// Created by andy on 2020/6/1.
//

//
#define BOOST_TEST_MODULE InetAddress_unittest
#include <net/InetAddress.h>
#include<boost/test/included/unit_test.hpp>

using namespace reactor;
using namespace reactor::net;

BOOST_AUTO_TEST_CASE(testInetAddress){
    InetAddress addr0(80);
    BOOST_CHECK_EQUAL(addr0.toIpPort(),string("0.0.0.0:80"));

    InetAddress addr1(1234,true);
    BOOST_CHECK_EQUAL(addr1.toIpPort(),string("127.0.0.1:1234"));

    InetAddress addr2("1.2.3.4",8080);
    BOOST_CHECK_EQUAL(addr2.toIpPort(),string("1.2.3.4:8080"));

    InetAddress addr3("255.254.253.252", 65535);
    BOOST_CHECK_EQUAL(addr3.toIpPort(), string("255.254.253.252:65535"));

}
