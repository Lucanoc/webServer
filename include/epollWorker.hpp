#pragma once

#include <cstddef>
#include <cstdio>
#include <functional>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <utility>
#include <fcntl.h>

#include "threadPool.hpp"
#include "connAccepter.hpp"
#include "httpWorker.hpp"

constexpr size_t threadsNum {20};
constexpr size_t maxEventSize {1024};

class epollWorker {
public:
    explicit epollWorker(int workPort);

    void run();
private:
    void waitAndRead();
    void control(int operation, int sockFd, uint32_t event);
    void onListenFdRead();
    void onWorkFdRead(int workFd);

    int epFd;
    connAccepter ac;
    threadPool tp;
};

inline
epollWorker::epollWorker(int workPort) 
: epFd(epoll_create1(0)), ac(workPort), tp(threadsNum) {
    if (epFd == -1) {
        throw std::runtime_error("epoll_create() error in epollWorker.");
    }

    control(EPOLL_CTL_ADD, ac.getListenFd(), EPOLLIN);
}

inline
void epollWorker::run() {
    waitAndRead();
}   

inline
void epollWorker::control(int operation, int sockFd, uint32_t event) {
    epoll_event ev;
    ev.data.fd =sockFd;
    ev.events = event;

    if (epoll_ctl(epFd, operation, sockFd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl() error in epollWorker.");
    }
}

inline
void epollWorker::waitAndRead() {
    epoll_event events[maxEventSize];

    while (true) {
        int num {epoll_wait(epFd, events, maxEventSize, -1)};

        for (int i {}; i != num; ++i) {
            if (events[i].data.fd == ac.getListenFd()) {
                onListenFdRead();
            } else {
                onWorkFdRead(events[i].data.fd);
            }
        }
    }
}

inline
void epollWorker::onListenFdRead() {
    std::pair<int, sockaddr_in> sockInfo(ac.accept(false));

    control(EPOLL_CTL_ADD, sockInfo.first, EPOLLIN | EPOLLET);
}

inline
void epollWorker::onWorkFdRead(int workFd) {
    control(EPOLL_CTL_DEL, workFd, 0);

    tp.submit([workFd, this]{
        httpWorkder(workFd).run();
    });
}