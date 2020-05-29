// Copyright (C) 2001-2003
// William E. Kempf
// Copyright (C) 2007 Anthony Williams
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE Condition_unittest

#include "base/Condition.h"
#include "base/Thread.h"
#include "base/Mutex.h"
#include <boost/test/included/unit_test.hpp>

using namespace reactor;
struct condition_test_data
{
  condition_test_data() :mutex_(),cond_(mutex_),notified_(0), awoken_(0) { }

  MutexLock mutex_;
  Condition cond_;
  int notified_;
  int awoken_;
}data;

void condition_test_thread()
{
  MutexLockGuard lock(data.mutex_);
  BOOST_CHECK(data.mutex_.isLockedByThisThread());
  while (!(data.notified_ > 0))
    data.cond_.wait();
  BOOST_CHECK(data.mutex_.isLockedByThisThread());
  data.awoken_++;
}


void condition_test_waits()
{
  MutexLockGuard lock(data.mutex_);
  BOOST_CHECK(data.mutex_.isLockedByThisThread());

  // Test wait.
  while (data.notified_ != 1)
    data.cond_.wait();
  BOOST_CHECK(data.mutex_.isLockedByThisThread());
  BOOST_CHECK_EQUAL(data.notified_, 1);
  data.awoken_++;
  data.cond_.notify();

  // Test wait.
  while (data.notified_ != 2)
    data.cond_.wait();
  BOOST_CHECK(data.mutex_.isLockedByThisThread());
  BOOST_CHECK_EQUAL(data.notified_, 2);
  data.awoken_++;
  data.cond_.notify();
  // Test timed_wait.
  int xt = 10;
  while (data.notified_ != 3)
    data.cond_.waitForSeconds(xt);
  BOOST_CHECK(data.mutex_.isLockedByThisThread());
  BOOST_CHECK_EQUAL(data.notified_, 3);
  data.awoken_++;
  data.cond_.notify();

}

void do_test_cond__waits()
{

  Thread thread(condition_test_waits);
  thread.start();
  {
    MutexLockGuard lock(data.mutex_);
    BOOST_CHECK(data.mutex_.isLockedByThisThread());

    sleep(1);
    data.notified_++;
    data.cond_.notify();
    while (data.awoken_ != 1)
      data.cond_.wait();
    BOOST_CHECK(data.mutex_.isLockedByThisThread());
    BOOST_CHECK_EQUAL(data.awoken_, 1);

    sleep(1);
    data.notified_++;//2
    data.cond_.notify();
    while (data.awoken_ != 2)
      data.cond_.wait();
    BOOST_CHECK(data.mutex_.isLockedByThisThread());
    BOOST_CHECK_EQUAL(data.awoken_, 2);

    sleep(20);
    data.notified_++;
    data.cond_.notify();
    while (data.awoken_ != 3)
      data.cond_.wait();
    BOOST_CHECK(data.mutex_.isLockedByThisThread());
    BOOST_CHECK_EQUAL(data.awoken_, 3);


  }

  thread.join();
  BOOST_CHECK_EQUAL(data.awoken_, 3);
}

BOOST_AUTO_TEST_CASE(cond_wait_test){
  do_test_cond__waits();
}