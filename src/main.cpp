#include "epollWorker.hpp"
#include "httpWorker.hpp"
#include "connAccepter.hpp"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <filesystem>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

auto main(int argc, char const ** argv) -> int {
    if (argc < 3) {
        std::cerr << "usage: appname [port] [work directory]\n";

        return 0;
    }

    chdir(argv[2]);

    epollWorker(atoi(argv[1])).run();

    // connAccepter connAc(atoi(argv[1]));

    // while (true) {
    //     std::pair<int, sockaddr_in> sockInfo(connAc.accept(false));

    //     httpWorkder(sockInfo.first).run();
    // }

    // int sock {socket(PF_INET, SOCK_STREAM, 0)};

    // if (sock == -1) {
    //     std::cerr << "socket() error";

    //     return 0;
    // }

    // sockaddr_in sockAddr;
    // bzero(&sockAddr, sizeof(sockAddr));
    // sockAddr.sin_family = AF_INET;
    // sockAddr.sin_port = htons(atoi(argv[1]));
    // sockAddr.sin_addr.s_addr = INADDR_ANY;

    // if (bind(sock, (sockaddr*)&sockAddr, sizeof (sockAddr)) == -1) {
    //     std::cerr << "bind() error";

    //     return 0;
    // }

    // if (listen(sock, 5) == -1) {
    //     std::cerr << "listen() error";
    
    //     return 0;
    // }

    // while (true) {
    //     sockaddr_in clntAddr;
    //     socklen_t addrLen;

    //     int clnt = accept(sock, (sockaddr*)&clntAddr, &addrLen);

    //     if (clnt == -1) {
    //         std::cerr << "accept() error";

    //         return 0;
    //     }

    //     int flag {fcntl(clnt, F_GETFL)};
    //     flag |= O_NONBLOCK;
    //     fcntl(clnt, F_SETFL, flag);

    //     httpWorkder(clnt).run();
    // }

    return 0;
}