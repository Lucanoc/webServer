#pragma once

#include <stdexcept>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <utility>
#include <fcntl.h>

class connAccepter {
public:
    explicit connAccepter(uint16_t workPort, int listenNum = 5);

    auto getListenFd() const -> int;
    auto accept() -> std::pair<int, sockaddr_in>;
private:
    int listenFd;
};

inline
connAccepter::connAccepter(uint16_t workPort, int listenNum)
: listenFd(socket(PF_INET, SOCK_STREAM,0)) {
    if (listenFd == -1) {
        throw std::runtime_error("socket() error in constructor of connAccepter.");
    }

    int flag {fcntl(listenFd, F_GETFL)};
    flag |= O_NONBLOCK;
    fcntl(listenFd, F_SETFL, flag);

    sockaddr_in listenAddr {};
    bzero(&listenAddr, sizeof(listenAddr));
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(workPort);
    listenAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenFd, (sockaddr*)&listenAddr, sizeof(listenAddr)) == -1) {
        throw std::runtime_error("bind() error in constructor of connAccepter.");
    }

    if (listen(listenFd, listenNum) == -1) {
        throw std::runtime_error("listen() error in constructor of connAccepter.");
    }
}

inline
auto connAccepter::getListenFd() const -> int {
    return listenFd;
}

inline
auto connAccepter::accept() -> std::pair<int, sockaddr_in> {
    std::pair<int, sockaddr_in> sockInfo;
    socklen_t addrLen;

    sockInfo.first = {::accept(listenFd, (sockaddr*)&sockInfo.second, &addrLen)};

    if (sockInfo.first == -1) {
        throw std::runtime_error("accept() error in connAccepter.");
    }

    return sockInfo;
}