// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)
#ifndef WEBSERVER_LOGGING_H
#define WEBSERVER_LOGGING_H

#include "base/LogStream.h"
#include "base/Timestamp.h"

namespace reactor {

    class TimeZone;

    class Logger {
    public:
        enum LogLevel {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        // compile time calculation of basename of source file
        class SourceFile {
        public:
            template<int N>
            SourceFile(const char (&arr)[N])
                    : data_(arr),
                      size_(N - 1) {
                const char *slash = strrchr(data_, '/'); // builtin function
                if (slash) {
                    data_ = slash + 1;
                    size_ -= static_cast<int>(data_ - arr);
                }
            }

            explicit SourceFile(const char *filename)
                    : data_(filename) {
                const char *slash = strrchr(filename, '/');
                if (slash) {
                    data_ = slash + 1;
                }
                size_ = static_cast<int>(strlen(data_));
            }

            const char *data_;
            int size_;
        };

        Logger(SourceFile file, int line);

        Logger(SourceFile file, int line, LogLevel level);

        Logger(SourceFile file, int line, LogLevel level, const char *func);

        Logger(SourceFile file, int line, bool toAbort);

        ~Logger();

        LogStream &stream() { return impl_.stream_; }

        static LogLevel logLevel();

        static void setLogLevel(LogLevel level);

        typedef void (*OutputFunc)(const char *msg, int len);

        typedef void (*FlushFunc)();

        static void setOutput(OutputFunc);

        static void setFlush(FlushFunc);

        static void setTimeZone(const TimeZone &tz);

    private:

        class Impl {
        public:
            typedef Logger::LogLevel LogLevel;

            Impl(LogLevel level, int old_errno, const SourceFile &file,
                 int line);

            void formatTime();

            void finish();

            Timestamp time_;
            LogStream stream_;
            LogLevel level_;
            int line_;
            SourceFile basename_;
        };

        Impl impl_;

    };

    extern Logger::LogLevel g_logLevel;

    inline Logger::LogLevel Logger::logLevel() {
        return g_logLevel;
    }

//
// CAUTION: do not write:
//
// if (good)
//   LOG_INFO << "Good news";
// else
//   LOG_WARN << "Bad news";
//
// this expends to
//
// if (good)
//   if (logging_INFO)
//     logInfoStream << "Good news";
//   else
//     logWarnStream << "Bad news";
//
#define LOG_TRACE if (reactor::Logger::logLevel() <= reactor::Logger::TRACE) \
  reactor::Logger(__FILE__, __LINE__, reactor::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (reactor::Logger::logLevel() <= reactor::Logger::DEBUG) \
  reactor::Logger(__FILE__, __LINE__, reactor::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (reactor::Logger::logLevel() <= reactor::Logger::INFO) \
  reactor::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN reactor::Logger(__FILE__, __LINE__, reactor::Logger::WARN).stream()
#define LOG_ERROR reactor::Logger(__FILE__, __LINE__, reactor::Logger::ERROR).stream()
#define LOG_FATAL reactor::Logger(__FILE__, __LINE__, reactor::Logger::FATAL).stream()
#define LOG_SYSERR reactor::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL reactor::Logger(__FILE__, __LINE__, true).stream()

    const char *strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
  ::reactor::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
    template<typename T>
    T *
    CheckNotNull(Logger::SourceFile file, int line, const char *names, T *ptr) {
        if (ptr == NULL) {
            Logger(file, line, Logger::FATAL).stream() << names;
        }
        return ptr;
    }

}  // namespace reactor

#endif //WEBSERVER_LOGGING_H
