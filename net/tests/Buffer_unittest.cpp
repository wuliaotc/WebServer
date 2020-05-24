//
// Created by andy on 2020/5/24.
//

#define BOOST_TEST_MODULE Buffer_unittest

#include"net/Buffer.h"
#include<boost/test/included/unit_test.hpp>

using std::string;
using reactor::net::Buffer;
const size_t Buffer::kCheapPrepend ;
const size_t Buffer::kInitialSize ;
/*
 *
 * */
BOOST_AUTO_TEST_CASE(testBufferAppendRetrieve) {
    Buffer buf;
    BOOST_CHECK_EQUAL(buf.readableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize);
    BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);

    string str(200, 'x');
    //no grow an no move
    BOOST_CHECK_GE(buf.writableBytes(), str.size());
    buf.append(str);
    BOOST_CHECK_EQUAL(buf.readableBytes(), str.size());
    BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize - str.size());
    BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);

    const string str2 = buf.retrieveAsString(50);
    BOOST_CHECK_EQUAL(str2.size(), 50);
    BOOST_CHECK_EQUAL(buf.readableBytes(), 200-str2.size());
    BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize - str.size());
    BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend+str2.size());
    BOOST_CHECK_EQUAL(str2, string(50, 'x'));

    buf.append(str);
    BOOST_CHECK_EQUAL(buf.readableBytes(), 2 * str.size() - str2.size());
    BOOST_CHECK_EQUAL(buf.writableBytes(),
                      Buffer::kInitialSize - 2 * str.size());
    BOOST_CHECK_EQUAL(buf.prependableBytes(),
                      Buffer::kCheapPrepend + str2.size());

    const string str3 = buf.retrieveAllAsString();
    BOOST_CHECK_EQUAL(str3.size(), 350);
    BOOST_CHECK_EQUAL(buf.readableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize);
    BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);
    BOOST_CHECK_EQUAL(str3, string(350, 'x'));
}

BOOST_AUTO_TEST_CASE(testBufferGrow) {
    Buffer buf;

    string str(Buffer::kInitialSize,'x' );
    buf.append(str);
    BOOST_CHECK_EQUAL(str.size(), buf.readableBytes());
    BOOST_CHECK_EQUAL(buf.writableBytes(), 0);

    buf.retrieve(Buffer::kInitialSize-1);//no grow
    BOOST_CHECK_EQUAL(buf.readableBytes(), 1);
    BOOST_CHECK_EQUAL(buf.writableBytes(), 0);

    string str2(Buffer::kInitialSize+Buffer::kCheapPrepend,'x');
    buf.append(str2);
    // grow but not move readable data
    // FIXME:move readable data
    BOOST_CHECK_EQUAL(buf.readableBytes(), str2.size()+1);
    BOOST_CHECK_EQUAL(buf.writableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.prependableBytes(),
                      Buffer::kCheapPrepend + Buffer::kInitialSize-1);

    buf.retrieveAll();
    BOOST_CHECK_EQUAL(buf.readableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.writableBytes(), str2.size()+ Buffer::kInitialSize);
    BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);
}

BOOST_AUTO_TEST_CASE(testBufferInsiderGrow) {
    Buffer buf;

    string str(Buffer::kInitialSize,'x' );
    buf.append(str);
    BOOST_CHECK_EQUAL(str.size(), buf.readableBytes());
    BOOST_CHECK_EQUAL(buf.writableBytes(), 0);

    buf.retrieve(Buffer::kInitialSize-1);
    BOOST_CHECK_EQUAL(buf.readableBytes(), 1);
    BOOST_CHECK_EQUAL(buf.writableBytes(), 0);

    string str2(Buffer::kInitialSize-1,'x');
    buf.append(str2);
    // no grow
    BOOST_CHECK_EQUAL(buf.readableBytes()-1, str2.size());
    BOOST_CHECK_EQUAL(buf.writableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.prependableBytes(),Buffer::kCheapPrepend);

    buf.retrieveAll();
    BOOST_CHECK_EQUAL(buf.readableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize);
    BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);
}
BOOST_AUTO_TEST_CASE(testBufferShrink)
{
    Buffer buf;
    buf.append(string(2000, 'y'));
    BOOST_CHECK_EQUAL(buf.readableBytes(), 2000);
    BOOST_CHECK_EQUAL(buf.writableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);

    buf.retrieve(1500);
    BOOST_CHECK_EQUAL(buf.readableBytes(), 500);
    BOOST_CHECK_EQUAL(buf.writableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend+1500);

    buf.shrink(0);
    BOOST_CHECK_EQUAL(buf.readableBytes(), 500);
    BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize-500);
    BOOST_CHECK_EQUAL(buf.retrieveAllAsString(), string(500, 'y'));
    BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);
}