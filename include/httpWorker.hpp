#pragma once

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <filesystem>
#include <sys/sendfile.h>

constexpr ssize_t maxRecvSize {128};

class httpWorkder {
public:
    ~httpWorkder();
    explicit httpWorkder(int workSocket_);

    void run();
private:
    void recv();
    void getUrl(char const * buffer);

    void send();
    void sendHttpHead();
    void sendHtml();

    int workSocket;
    std::string filePath;
    bool working;
    bool httpOk;

    inline static std::string const badReq {
        "HTTP/1.1 404 Not Found\r\n"
        "content-type: text/html; charset=utf-8\r\n"
        "content-length: -1\r\n"
        "\r\n"
    };
    inline static std::string const okReq {
        "HTTP/1.1 200 OK\r\n"
        "content-type: text/html; charset=utf-8\r\n"
        "content-length: -1\r\n"
        "\r\n"
    };
};

inline
httpWorkder::~httpWorkder() {
    close(workSocket);
}

inline
httpWorkder::httpWorkder(int workSocket_)
: workSocket(workSocket_), filePath("index.html"), working(false), httpOk(true) {}

inline
void httpWorkder::run() {
    recv();

    if (working) {
        send();
    }
}

inline
void httpWorkder::recv() {
    char buffer[maxRecvSize] {};
    ssize_t recvLen {};
    ssize_t eachLen {};

    while ((eachLen = ::recv(workSocket, buffer + recvLen, maxRecvSize, 0)) != 0) {
        if (eachLen == -1) {
            if (errno == EAGAIN) {
               continue;
            } else {
                break;
            }
        }

        recvLen += eachLen;

        if (recvLen > maxRecvSize - 1) {
            working = true;

            break;
        }
    }

    buffer[maxRecvSize - 1] = '\0';

    if (working) {
        getUrl(buffer);
    }
}

inline
void httpWorkder::getUrl(char const * buffer) {
    char method[8] {};
    char url[64] {};

    sscanf(buffer, "%[^ ] /%[^ ]", method, url);

    if (strcmp(method, "GET") != 0 && strcmp(method, "get") != 0) {
        return;
    }

    std::cout << "req url : " << url << '\n';

    if (std::filesystem::exists(url)) {
        filePath = url;
    } else {
        if (strcmp(url, "") != 0) {
            filePath = "404.html";
            
            httpOk = false;
        }
    }
}

inline
void httpWorkder::send() {
    sendHttpHead();
    sendHtml();
}

inline
void httpWorkder::sendHttpHead () {
    ::send(workSocket, 
    httpOk ? okReq.c_str() : badReq.c_str(), 
    httpOk ? okReq.size() : badReq.size(), 
    0);
}

inline
void httpWorkder::sendHtml() {
    int htmlFile {::open(filePath.c_str(), O_RDONLY)};

    __off_t srcSize {::lseek(htmlFile, 0, SEEK_END)};
    ::lseek(htmlFile, 0, SEEK_SET);

    off_t offSet {};

    while (offSet < srcSize) {
        ::sendfile(workSocket, htmlFile, &offSet, srcSize);
    }

    close(htmlFile);
}