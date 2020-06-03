//
// Created by andy on 20-5-24.
//
#include <iostream>
#include <unordered_map>
#include "net/http/HttpServer.h"
#include "net/http/HttpResponse.h"
#include "net/http/HttpRequest.h"
#include "net/EventLoop.h"
#include "base/Logging.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

using namespace reactor;
using namespace reactor::net;
constexpr bool benchmark = false;
extern char favicon[555];
enum MimeType {
    DOT,   //application/x-

    CSS,    // text/css
    HTML,    // text/html
    JS,    // application/x-javascript

    BMP,    // application/x-bmp
    ICO,    // image/x-icon
    JPE,    // image/jpeg
    JPEG,    // image/jpeg
    JPG,    // application/x-jpg

    MP4,     // video/mpeg4

    PDF,    // application/pdf
    UNKOWNMIME,
    MIME_NUM
};
const char *mimeStr[MIME_NUM] = {
        "application/x-",
        "text/css",
        "text/html",
        "application/x-javascript",
        "application/x-bmp",
        "image/x-icon",
        "image/jpeg",
        "image/jpeg",
        "application/x-jpg",
        "video/mpeg4",
        "application/pdf",
        "text/plain"
};
const std::unordered_map<std::string, MimeType> mimeMap
        {
                {".",     DOT},

                {".css",  CSS},
                {".html", HTML},
                {".js",   JS},

                {".bmp",  BMP},
                {".ico",  ICO},
                {".jpe",  JPE},
                {".jpeg", JPEG},
                {".jpg",  JPG},

                {".mp4",  MP4},
                {".pdf",  PDF}
        };

struct File {
    int fd;
    size_t size_;
    int64_t modifyTime_;
    Buffer buf_;
    MimeType mime_;

    File()
            : fd(-1),
              size_(0),
              modifyTime_(0),
              buf_(),
              mime_(UNKOWNMIME) {}
};

std::unordered_map<std::string, File> fileMap;

void onRequest(const HttpRequest &req, HttpResponse *resp) {
    std::cout << "Headers " << req.methodString() << " " << req.path()
              << std::endl;
    if (!benchmark) {
        const std::map<string, string> &headers = req.headers();
        for (const auto &header : headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }
    string path = req.path();
    assert(path.size()>0);
    if (path[path.size()-1]=='/') {
        //FIXME:hack
        path += "index.html";
        if (faccessat(AT_FDCWD, path.c_str()+1, R_OK, 0) == 0) {
            LOG_INFO<<path<<" existed,will change to index file";
        } else {
            LOG_INFO<<path<<" not existed,resp defalut home";
            resp->setStatusCode(HttpResponse::k200Ok);
            resp->setStatusMessage("OK");
            resp->setContentType("text/html");
            resp->addHeader("Server", "Muduo");
            string now = Timestamp::now().toFormattedString();
            resp->setBody("<html><head><title>This is title</title></head>"
                          "<body><h1>Hello</h1>Now is " + now +
                          "</body></html>");
            return ;
        }

    }

    if (path == "/favicon.ico") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("image/png");
        resp->setBody(string(favicon, sizeof favicon));
    } else if (path == "/hello") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/plain");
        resp->addHeader("Server", "Muduo");
        resp->setBody("hello, world!\n");
    } else if (std::find(path.begin(), path.end(), '.') - path.begin() !=
               1) {// /. /..
        //FIXME:hack
        // and it must be a small file
        if (fileMap.find(path) == fileMap.end() &&
            ::faccessat(AT_FDCWD, path.c_str() + 1, R_OK, 0) == 0) {
            struct stat stat_;
            int fd = ::openat(AT_FDCWD, path.c_str() + 1, O_RDONLY);
            if (fd != -1 && fstat(fd, &stat_) == 0) {
                File file;
                file.fd = fd;
                file.size_ = stat_.st_size;
                file.modifyTime_ = stat_.st_mtim.tv_sec;
                size_t dot = path.rfind(".");
                auto it=mimeMap.find(path.substr(dot));
                if (it!=mimeMap.end()){
                    file.mime_=it->second;
                }
                fileMap[path]=file;
            }
        }
        if (fileMap.find(path) != fileMap.end()) {
            File &f = fileMap[path];
            int err = 0;
            if (f.fd != -1 && f.buf_.readableBytes() != f.size_) {
                size_t len = f.buf_.readFd(f.fd, &err);
                if (err != 0) {
                    LOG_ERROR << "readFd err:" << strerror_tl(err);
                }
                if (len != 0 && f.size_ == f.buf_.readableBytes()) {
                    LOG_INFO << "read " << f.size_ << "bytes finished,fd="
                             << f.fd;
                }
            }
            resp->setStatusCode(HttpResponse::k200Ok);
            resp->setStatusMessage("OK");
            resp->setContentType(mimeStr[f.mime_]);
            resp->addHeader("Server", "Muduo");
            resp->setBody(string(f.buf_.peek(), implicit_cast<const char *>(
                    f.buf_.beginWrite())));
        } else {
            resp->setStatusCode(HttpResponse::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }
    } else {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
}

void sendFile() {

}

int main() {
    EventLoop loop;
    InetAddress listenAddr(8080);
    HttpServer server_(&loop, listenAddr, 30);
    server_.setHttpCallback(
            std::bind(onRequest, std::placeholders::_1, std::placeholders::_2));

    //server_.setThreadNum(5);

    server_.start();
    loop.loop();

    return 0;
}

char favicon[555] = {
        '\x89', 'P', 'N', 'G', '\xD', '\xA', '\x1A', '\xA',
        '\x0', '\x0', '\x0', '\xD', 'I', 'H', 'D', 'R',
        '\x0', '\x0', '\x0', '\x10', '\x0', '\x0', '\x0', '\x10',
        '\x8', '\x6', '\x0', '\x0', '\x0', '\x1F', '\xF3', '\xFF',
        'a', '\x0', '\x0', '\x0', '\x19', 't', 'E', 'X',
        't', 'S', 'o', 'f', 't', 'w', 'a', 'r',
        'e', '\x0', 'A', 'd', 'o', 'b', 'e', '\x20',
        'I', 'm', 'a', 'g', 'e', 'R', 'e', 'a',
        'd', 'y', 'q', '\xC9', 'e', '\x3C', '\x0', '\x0',
        '\x1', '\xCD', 'I', 'D', 'A', 'T', 'x', '\xDA',
        '\x94', '\x93', '9', 'H', '\x3', 'A', '\x14', '\x86',
        '\xFF', '\x5D', 'b', '\xA7', '\x4', 'R', '\xC4', 'm',
        '\x22', '\x1E', '\xA0', 'F', '\x24', '\x8', '\x16', '\x16',
        'v', '\xA', '6', '\xBA', 'J', '\x9A', '\x80', '\x8',
        'A', '\xB4', 'q', '\x85', 'X', '\x89', 'G', '\xB0',
        'I', '\xA9', 'Q', '\x24', '\xCD', '\xA6', '\x8', '\xA4',
        'H', 'c', '\x91', 'B', '\xB', '\xAF', 'V', '\xC1',
        'F', '\xB4', '\x15', '\xCF', '\x22', 'X', '\x98', '\xB',
        'T', 'H', '\x8A', 'd', '\x93', '\x8D', '\xFB', 'F',
        'g', '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f', 'v',
        'f', '\xDF', '\x7C', '\xEF', '\xE7', 'g', 'F', '\xA8',
        '\xD5', 'j', 'H', '\x24', '\x12', '\x2A', '\x0', '\x5',
        '\xBF', 'G', '\xD4', '\xEF', '\xF7', '\x2F', '6', '\xEC',
        '\x12', '\x20', '\x1E', '\x8F', '\xD7', '\xAA', '\xD5', '\xEA',
        '\xAF', 'I', '5', 'F', '\xAA', 'T', '\x5F', '\x9F',
        '\x22', 'A', '\x2A', '\x95', '\xA', '\x83', '\xE5', 'r',
        '9', 'd', '\xB3', 'Y', '\x96', '\x99', 'L', '\x6',
        '\xE9', 't', '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',
        '\xA7', '\xC4', 'b', '1', '\xB5', '\x5E', '\x0', '\x3',
        'h', '\x9A', '\xC6', '\x16', '\x82', '\x20', 'X', 'R',
        '\x14', 'E', '6', 'S', '\x94', '\xCB', 'e', 'x',
        '\xBD', '\x5E', '\xAA', 'U', 'T', '\x23', 'L', '\xC0',
        '\xE0', '\xE2', '\xC1', '\x8F', '\x0', '\x9E', '\xBC', '\x9',
        'A', '\x7C', '\x3E', '\x1F', '\x83', 'D', '\x22', '\x11',
        '\xD5', 'T', '\x40', '\x3F', '8', '\x80', 'w', '\xE5',
        '3', '\x7', '\xB8', '\x5C', '\x2E', 'H', '\x92', '\x4',
        '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g', '\x98',
        '\xE9', '6', '\x1A', '\xA6', 'g', '\x15', '\x4', '\xE3',
        '\xD7', '\xC8', '\xBD', '\x15', '\xE1', 'i', '\xB7', 'C',
        '\xAB', '\xEA', 'x', '\x2F', 'j', 'X', '\x92', '\xBB',
        '\x18', '\x20', '\x9F', '\xCF', '3', '\xC3', '\xB8', '\xE9',
        'N', '\xA7', '\xD3', 'l', 'J', '\x0', 'i', '6',
        '\x7C', '\x8E', '\xE1', '\xFE', 'V', '\x84', '\xE7', '\x3C',
        '\x9F', 'r', '\x2B', '\x3A', 'B', '\x7B', '7', 'f',
        'w', '\xAE', '\x8E', '\xE', '\xF3', '\xBD', 'R', '\xA9',
        'd', '\x2', 'B', '\xAF', '\x85', '2', 'f', 'F',
        '\xBA', '\xC', '\xD9', '\x9F', '\x1D', '\x9A', 'l', '\x22',
        '\xE6', '\xC7', '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15',
        '\x90', '\x7', '\x93', '\xA2', '\x28', '\xA0', 'S', 'j',
        '\xB1', '\xB8', '\xDF', '\x29', '5', 'C', '\xE', '\x3F',
        'X', '\xFC', '\x98', '\xDA', 'y', 'j', 'P', '\x40',
        '\x0', '\x87', '\xAE', '\x1B', '\x17', 'B', '\xB4', '\x3A',
        '\x3F', '\xBE', 'y', '\xC7', '\xA', '\x26', '\xB6', '\xEE',
        '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
        '\xA', '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X', '\x0',
        '\x27', '\xEB', 'n', 'V', 'p', '\xBC', '\xD6', '\xCB',
        '\xD6', 'G', '\xAB', '\x3D', 'l', '\x7D', '\xB8', '\xD2',
        '\xDD', '\xA0', '\x60', '\x83', '\xBA', '\xEF', '\x5F', '\xA4',
        '\xEA', '\xCC', '\x2', 'N', '\xAE', '\x5E', 'p', '\x1A',
        '\xEC', '\xB3', '\x40', '9', '\xAC', '\xFE', '\xF2', '\x91',
        '\x89', 'g', '\x91', '\x85', '\x21', '\xA8', '\x87', '\xB7',
        'X', '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N', 'N',
        'b', 't', '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
        '\xEC', '\x86', '\x2', 'H', '\x26', '\x93', '\xD0', 'u',
        '\x1D', '\x7F', '\x9', '2', '\x95', '\xBF', '\x1F', '\xDB',
        '\xD7', 'c', '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF',
        '\x22', 'J', '\xC3', '\x87', '\x0', '\x3', '\x0', 'K',
        '\xBB', '\xF8', '\xD6', '\x2A', 'v', '\x98', 'I', '\x0',
        '\x0', '\x0', '\x0', 'I', 'E', 'N', 'D', '\xAE',
        'B', '\x60', '\x82',
};
