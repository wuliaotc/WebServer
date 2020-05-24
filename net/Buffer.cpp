//
// Created by andy on 20-5-23.
//

#include "net/Buffer.h"
#include<sys/uio.h>

using namespace reactor;
using namespace reactor::net;

const char Buffer::kCRLF[] = "\r\n";

ssize_t Buffer::readFd(int fd, int *savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writeable = writableBytes();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const ssize_t n = readv(fd, vec, 2);
    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writeable) {
        writerIndex_ += n;
    } else {//use extrabuf
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writeable);
    }
    return n;
}