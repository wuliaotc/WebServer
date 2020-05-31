//
// Created by andy on 20-5-23.
//
#include "EventLoop.h"

#include <assert.h>
#include <poll.h>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "base/Logging.h"
#include "base/Thread.h"
#include "net/Channel.h"
#include "net/Poller.h"
#include "net/TimerQueue.h"

using namespace reactor;
using namespace reactor::net;
__thread EventLoop *t_loopInThisThread = 0;

class IgnoreSigPipe {
public:
  IgnoreSigPipe() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
      abort();
  }
};

const int kPollTimeMs = 10000;

static int createEventfd() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_SYSERR << " Failed in eventfd";
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop()
    : looping_(false), quit_(false), threadId_(CurrentThread::tid()),
      poller_(new Poller(this)), timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()), wakeupChannel_(new Channel(this, wakeupFd_)) {
  LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  assert(!looping_);
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;

  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    for (ChannelList::iterator it = activeChannels_.begin();
         it != activeChannels_.end(); ++it) {
      (*it)->handleEvent(pollReturnTime_);
    }
    doPendingFunctors();
  }
  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (isInLoopThread()) {
    wakeup();
  }
  // wakeup()
  /*
  quit只是设置标记,如果EventLoop被阻塞,可能延迟很高
  将来使用wakeup唤醒线程
  */
}

void EventLoop::updateChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " << CurrentThread::tid();
}

TimerId EventLoop::runAt(const reactor::Timestamp &time,
                         const TimerCallback &cb) {
  // interval =0 ->> no repeat
  return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb) {
  reactor::Timestamp time(reactor::addTime(reactor::Timestamp::now(), delay));
  return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb) {
  reactor::Timestamp time(addTime(reactor::Timestamp::now(), interval));
  return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::queueInLoop(const Functor &cb) {
  MutexLockGuard lock(mutex_);
  pendingFunctors_.push_back(cb);
  if (!isInLoopThread() || callingPendingFunctors_) {
    //只有io线程调用queueInLoop才不需要唤醒
    //当当前线程时,说明该函数是在loop()内被调用的,不需要唤醒
    //同时如果是callingPendingfunctors,说明该函数正在被dopedingfunctor调用
    // TODO
    wakeup();
  }
}

void EventLoop::runInLoop(const Functor &cb) {
  if (isInLoopThread())
    cb(); //如果在当前线程调用runInLoop 同步执行
  else
    queueInLoop(cb);
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;
  {
    MutexLockGuard lock(mutex_);
    /*
     * 使用栈上对象,避免使用锁来保护,同时避免死锁,
     * 因为functors也可能调用doPendfunc 如果使用锁就会死锁
     * */
    functors.swap(pendingFunctors_);
  }
  for (size_t i = 0; i < functors.size(); ++i) {

    functors[i]();
  }
  callingPendingFunctors_ = false;
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t ret = ::read(wakeupFd_, &one, sizeof(one));
  if (ret != sizeof(one)) {
    LOG_ERROR << "EventLoop::handleRead() read " << ret
              << " bytes instead of 8";
  }
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t ret = write(wakeupFd_, &one, sizeof(one));
  if (ret != sizeof(one)) {
    LOG_ERROR << "EventLoop::akeup() writes " << ret << " bytes instead of 8";
  }
}

void EventLoop::cancel(TimerId timerId) {
  runInLoop(std::bind(&TimerQueue::cancel, timerQueue_.get(), timerId));
}
