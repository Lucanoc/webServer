#pragma once

#include <functional>
#include <map>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <utility>
#include <fcntl.h>

#include "connAccepter.hpp"

class epollWorker {
public:
    explicit epollWorker(connAccepter & ac_);

    void wait();
private:
    void control(int operation, int sockFd, uint32_t event);

    void onListenFdRead();
    void onWorkFdRead(int workFd);

    int epFd;
    connAccepter & ac;
};

inline
epollWorker::epollWorker(connAccepter & ac_) 
: epFd(epoll_create(1)), ac(ac_) {
    if (epFd == -1) {
        throw std::runtime_error("epoll_create() error in epollWorker.");
    }

    control(EPOLL_CTL_ADD, ac.getListenFd(), EPOLLIN | EPOLLET);
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

void epollWorker::wait() {
    epoll_event events[1024];

    while (true) {
        int num {epoll_wait(epFd, events, 1024, -1)};

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
}

inline
void epollWorker::onWorkFdRead(int workFd) {
    
}