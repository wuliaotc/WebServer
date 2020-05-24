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
                assert(writeableBytes() == kInitialSize);
                assert(prependableBytes() == kCheapPrepend);
            }

            void swap(Buffer &rhs) {
                buffer_.swap(rhs.buffer_);
                ::std::swap(readerIndex_, rhs.readerIndex_);
                ::std::swap(writerIndex_, rhs.writerIndex_);
            }

            size_t readableBytes() { return writerIndex_ - readerIndex_; }

            size_t writeableBytes() { return buffer_.size() - writerIndex_; }

            size_t prependableBytes() { return readerIndex_; }

            //first readable char ptr
            const char *peek() const { return begin() + readerIndex_; }
            const char* findCRLF() const
            {
                // FIXME: replace with memmem()?
                const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
                return crlf == beginWrite() ? NULL : crlf;
            }
            //finish read len
            void retrieve(size_t len) {
                assert(len <= readableBytes());
                readerIndex_ += len;
            }

            void retrieveUntil(const char *end) {
                assert(peek() <= end);
                assert(end <= beginWrite());
                retrieve(end - peek());
            }

            void retrieveAll() {
                readerIndex_ = kCheapPrepend;
                writerIndex_ = kCheapPrepend;
            }

            std::string retrieveAsString() {
                std::string str(peek(), readableBytes());
                retrieveAll();
                return str;
            }

            void append(const ::std::string &str) {
                append(str.data(), str.length());
            }

            void append(const char *data, size_t len) {
                ensureWriteableBytes(len);
                ::std::copy(data, data + len, beginWrite());
                hasWritten(len);
            }

            void append(const void *data, size_t len) {
                append(static_cast<const char *>(data), len);
            }

            void ensureWriteableBytes(size_t len) {
                if (writeableBytes() < len) {
                    makeSpace(len);
                }
                assert(writeableBytes() >= len);
            }

            char *beginWrite() { return begin() + writerIndex_; }

            const char *beginWrite() const { return begin() + writerIndex_; }

            void hasWritten(size_t len) { writerIndex_ += len; }

            void prepend(const void *data, size_t len) {
                assert(len <= prependableBytes());
                readerIndex_ -= len;
            }

            void shrink(size_t reserve) {
                std::vector<char> buf(
                        kCheapPrepend + readableBytes() + reserve);
                std::copy(peek(), peek() + readableBytes(),
                          buf.begin() + kCheapPrepend);
                buf.swap(buffer_);
            }

            ssize_t readFd(int fd, int *savedErrno);

        private:
            char *begin() { return &(*buffer_.begin()); }

            const char *begin() const { return &(*buffer_.begin()); }

            void makeSpace(size_t len) {
                if (writeableBytes() + prependableBytes() < len) {
                    buffer_.resize(writerIndex_ + len);
                } else {
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
