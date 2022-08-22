#pragma once

#include <cstddef>
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

#include <co/log.h>

#include "threadPool.hpp"
#include "connAccepter.hpp"
#include "httpWorker.hpp"

constexpr size_t threadsNum {20};
constexpr size_t maxEventSize {1024};

class epollWorker {
public:
    explicit epollWorker(connAccepter ac_);

    template <typename FUNC>
    void run(FUNC && func);
private:
    void wait();
    void control(int operation, int sockFd, uint32_t event);
    void onListenFdRead();
    void onWorkFdRead(int workFd);

    int epFd;
    connAccepter ac;
    std::function<void(int)> todo;
    threadPool tp;
};

inline
epollWorker::epollWorker(connAccepter ac_) 
: epFd(epoll_create(1)), ac(ac_), tp(threadsNum) {
    if (epFd == -1) {
        throw std::runtime_error("epoll_create() error in epollWorker.");
    }

    control(EPOLL_CTL_ADD, ac.getListenFd(), EPOLLIN);
}

template <typename FUNC>
void epollWorker::run(FUNC && func) {
    todo = std::forward<FUNC>(func);

    wait();
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
void epollWorker::wait() {
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
    std::pair<int, sockaddr_in> sockInfo(ac.accept());

    int flag {fcntl(sockInfo.first, F_GETFL)};
    flag |= O_NONBLOCK;
    fcntl(sockInfo.first, F_SETFL, flag);

    control(EPOLL_CTL_ADD, sockInfo.first, EPOLLIN | EPOLLET);

    std::string addr(inet_ntoa(sockInfo.second.sin_addr));
    LOG << "connecting: "
        << "socket " << sockInfo.first
        << " addr " << addr
        << " port " << ntohs(sockInfo.second.sin_port);
}

inline
void epollWorker::onWorkFdRead(int workFd) {
    control(EPOLL_CTL_DEL, workFd, 0);

    tp.submit([workFd, this]{
        todo(workFd);
    });

    LOG << "removing: " << "socket " << workFd;
}