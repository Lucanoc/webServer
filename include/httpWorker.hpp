#pragma once

#include <bits/types/FILE.h>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
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
};

inline
httpWorkder::~httpWorkder() {
    close(workSocket);
}

inline
httpWorkder::httpWorkder(int workSocket_)
: workSocket(workSocket_), filePath("index.html"), working(false) {}

inline
void httpWorkder::run() {
    recv();

    std::cout << "\n ready to send()\n";

    std::cout << "working: " << working << '\n';

    if (working) {
        send();
    }

    std::cout << "\nfinish sending!\n";
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

    std::cout << buffer << '\n';

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

    if (std::filesystem::exists(url)) {
        filePath = url;
    } else {
        if (strcmp(url, "") != 0) {
            filePath = "404.html";
        }
    }

    std::cout << "want url is : " << filePath << '\n';
}

inline
void httpWorkder::send() {
    sendHttpHead();
    sendHtml();
}

inline
void httpWorkder::sendHttpHead () {
    std::stringstream buffer;

    buffer << "HTTP/1.1 ";
    buffer << (filePath == "404.html" ? "404 Not Found\r\n" : "200 OK\r\n");
    buffer << "content-length: " << -1 << " \r\n";
    buffer << "\r\n";

    ::send(workSocket, buffer.str().c_str(), buffer.str().size(), 0);
    std::cout << "\nsendHttpHead " << buffer.str() << '\n';
}

inline
void httpWorkder::sendHtml() {
    int srcFd {::open(filePath.c_str(), O_RDONLY)};

    __off_t srcSize {::lseek(srcFd, 0, SEEK_END)};
    ::lseek(srcFd, 0, SEEK_SET);

    std::cout << '\n' << srcFd << ' ' << srcSize << '\n';
    std::cout << errno << '\n';

    off_t offSet {};

    while (offSet < srcSize) {
        std::cout << ::sendfile(workSocket, srcFd, &offSet, srcSize) << '\n';
    }

    close(srcFd);
}