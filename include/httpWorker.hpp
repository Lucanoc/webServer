#pragma once

#include "co/log.h"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <filesystem>
#include <sys/sendfile.h>

constexpr size_t maxBufferSize {2048};

class httpWorkder {
public:
    enum class workStatus : uint8_t {
        waiting,
        connOver,
        recvErr,
        recvFinish,
        sendErr,
        sendFinish,
        methodErr
    };

    ~httpWorkder();

    explicit httpWorkder(int workSocket_);

    auto recv() -> workStatus;
    auto send() -> workStatus;

private:
    void analyse(char * buffer, ssize_t useLen);
    void disAnalyse();

    auto sendDir() -> workStatus;
    auto sendFile() -> workStatus;

    int workSocket;
    workStatus status;
    std::filesystem::path filePath;
};

inline
httpWorkder::~httpWorkder() {
    close(workSocket);
}

inline
httpWorkder::httpWorkder(int workSocket_)
: workSocket(workSocket_), status(workStatus::waiting) {}

inline
auto httpWorkder::recv() -> httpWorkder::workStatus {
    char buffer[maxBufferSize] {};
    ssize_t useLen {};
    ssize_t recvLen {};

    while ((recvLen = ::recv(workSocket, buffer + useLen, maxBufferSize, 0) != 0)) {
        if (recvLen == -1) {
            if (errno == EAGAIN) {
                status = workStatus::recvFinish;
            } else {
                status = workStatus::recvErr;
            }

            break;
        }

        useLen += recvLen;

        if (maxBufferSize < useLen) {
            status = workStatus::recvFinish;

            break;
        }
    }

    LOG << "socket: " << workSocket << " recv() over";

    if (status == workStatus::recvFinish) {
        analyse(buffer, useLen);
    }

    if (status == workStatus::waiting) {
        status = workStatus::connOver;
    }

    return status;
}

inline
auto httpWorkder::send() -> httpWorkder::workStatus {
    return std::filesystem::is_directory(filePath) ? sendDir() : sendFile();
}

inline
void httpWorkder::analyse(char * buffer, ssize_t useLen) {

    LOG << "socket: " << workSocket << " useLen: " << useLen <<" analysed!";

    char method[8] {};
    char url[128] {};
    char version[8] {};

    //GET /etc/apt/sources.txt HTTP/1.1
    sscanf(buffer, "%[^ ] %[^ ] %s", method, url, version);

    if (strcmp(method, "GET") != 0 && strcmp(method, "get") != 0) {
        status = workStatus::methodErr;

        return;
    }

    filePath = "." + std::string(url);
}

inline
auto httpWorkder::sendDir() -> httpWorkder::workStatus {
    

    return status;
}

inline
auto httpWorkder::sendFile() -> httpWorkder::workStatus {
    // std::string buffer;

    // std::ifstream ifs(filePath);

    // if (!ifs.is_open()) {
    //     status = workStatus::sendErr;
    // } else {
    //     while (ifs >> buffer) {
    //         ::send(workSocket, buffer.c_str(), buffer.size(), 0);

    //         std::this_thread::sleep_for(std::chrono::microseconds(1));
    //     }

    //     status = workStatus::sendFinish;
    // }

    int srcFd {::open(filePath.c_str(), O_RDONLY)};

    if (srcFd < 0) {
        status = workStatus::sendErr;
    } else {
         __off_t srcSize {::lseek(srcFd, 0, SEEK_END)};

        ::sendfile(workSocket, srcFd, nullptr, srcSize);

        status = workStatus::sendFinish;
    }

    return status;
}