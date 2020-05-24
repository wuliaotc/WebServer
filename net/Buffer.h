//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_BUFFER_H
#define WEBSERVER_BUFFER_H

#include <assert.h>

#include <algorithm>
#include <string>
#include <vector>

namespace reactor {
///
/// +-------------------+-----------------+------------------+
/// | prependable bytes |  readable bytes | writeable bytes  |
//  |                   |                 |                  |
//  +-------------------+-----------------+------------------+
//  |                   |                 |                  |
//  0        <=     readerIndex  <=  writerIndex    <=    size
    namespace net {
        class Buffer {
        public:
            static const size_t kCheapPrepend = 8;
            static const size_t kInitialSize = 1024;

            Buffer()
                    : buffer_(kCheapPrepend + kInitialSize),
                      readerIndex_(kCheapPrepend),
                      writerIndex_(kCheapPrepend) {
                assert(readableBytes() == 0);
                assert(writableBytes() == kInitialSize);
                assert(prependableBytes() == kCheapPrepend);
            }

            void swap(Buffer &rhs) {
                buffer_.swap(rhs.buffer_);
                ::std::swap(readerIndex_, rhs.readerIndex_);
                ::std::swap(writerIndex_, rhs.writerIndex_);
            }

            // return readable bytes
            size_t readableBytes() { return writerIndex_ - readerIndex_; }

            // return writable bytes
            size_t writableBytes() { return buffer_.size() - writerIndex_; }

            // return available bytes for prepend
            size_t prependableBytes() { return readerIndex_; }

            //first readable char ptr
            const char *peek() const { return begin() + readerIndex_; }

            //find first CRLF position
            //return NULL if no any CRLF
            const char *findCRLF() const {
                // FIXME: replace with memmem()?
                const char *crlf = std::search(peek(), beginWrite(), kCRLF,
                                               kCRLF + 2);
                return crlf == beginWrite() ? NULL : crlf;
            }

            //finish read len read
            void retrieve(size_t len) {
                assert(len <= readableBytes());
                if (len < readableBytes()) {
                    readerIndex_ += len;
                } else {
                    retrieveAll();
                }

            }

            // set readable pointer to end
            // next read will begin from end
            void retrieveUntil(const char *end) {
                assert(peek() <= end);
                assert(end <= beginWrite());
                retrieve(end - peek());
            }

            // reset all
            // readableBytes =0,writableBytes =0
            void retrieveAll() {
                readerIndex_ = kCheapPrepend;
                writerIndex_ = kCheapPrepend;
            }

            // get all readable bytes
            // and reset Buffer
            std::string retrieveAllAsString() {
                std::string str(peek(), readableBytes());
                retrieveAll();
                return str;
            }

            std::string retrieveAsString(size_t len) {
                assert(len <= readableBytes());
                std::string str(peek(), len);
                retrieve(len);
                return str;
            }

            //append data
            void append(const std::string &str) {
                append(str.data(), str.length());
            }

            //append data
            void append(const char *data, size_t len) {
                ensureWriteableBytes(len);
                std::copy(data, data + len, beginWrite());
                hasWritten(len);
            }

            //append data
            void append(const void *data, size_t len) {
                append(static_cast<const char *>(data), len);
            }

            // ensure enough bytes to write len bytes
            // warning: if can't satisfied room,will abort
            void ensureWriteableBytes(size_t len) {
                if (writableBytes() < len) {
                    makeSpace(len);
                }
                assert(writableBytes() >= len);
            }

            // return a pointer point to the first writable bytes
            char *beginWrite() { return begin() + writerIndex_; }

            const char *beginWrite() const { return begin() + writerIndex_; }

            // move the writable pointer after write len bytes
            void hasWritten(size_t len) { writerIndex_ += len; }

            // prepend before read pointer
            // warning:ensure enough bytes to prepend
            // no enough room will call abort
            void prepend(const void *data, size_t len) {
                assert(len <= prependableBytes());
                readerIndex_ -= len;
                const char *d = static_cast<const char *>(data);
                std::copy(d, d + len, begin() + readerIndex_);
            }

            // move readable bytes to front
            // and ensure reserve for write
            //TODO:利用栈上对象还是原地迁移
            void shrink(size_t reserve) {
                int readable = readableBytes();

                std::copy(peek(), peek() + readable,
                          buffer_.begin() + kCheapPrepend);


                int newSize = kInitialSize + kCheapPrepend;
                if (kCheapPrepend + readable + reserve > newSize)
                    newSize = kCheapPrepend + readable + reserve;
                buffer_.resize(newSize);// O(n)
                readerIndex_ = kCheapPrepend;
                writerIndex_ = readerIndex_ + readable;
                buffer_.shrink_to_fit();//O(n)
            }

            // read from fd
            // @return n bytes ha been read
            // if n<0,some error occurred
            ssize_t readFd(int fd, int *savedErrno);

        private:
            // return buffer_.begin()
            char *begin() { return &(*buffer_.begin()); }

            const char *begin() const { return &(*buffer_.begin()); }

            // try to make more space to store len bytes
            void makeSpace(size_t len) {
//                assert(writableBytes()<len);
                if (writableBytes() + prependableBytes() <
                    len + kCheapPrepend) {
                    buffer_.resize(writerIndex_ + len);
                } else {
                    //move readable bytes to makespace
                    //no need to alloc memmory
                    assert(kCheapPrepend < readerIndex_);
                    size_t readable = readableBytes();
                    std::copy(begin() + readerIndex_, begin() + writerIndex_,
                              begin() + kCheapPrepend);
                    readerIndex_ = kCheapPrepend;
                    writerIndex_ = readable + readerIndex_;
                    assert(readable == readableBytes());
                }
            }

            std::vector<char> buffer_;
            size_t readerIndex_;
            size_t writerIndex_;

            static const char kCRLF[];
        };
    }//namespace net
}// namespace reactor

#endif //WEBSERVER_BUFFER_H
