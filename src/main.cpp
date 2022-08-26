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

    //connAccepter ca(atoi(argv[1]));

    int sock {socket(PF_INET, SOCK_STREAM, 0)};

        if (sock == -1) {
            std::cerr << "socket() error";

            return 0;
        }

        sockaddr_in sockAddr;
        bzero(&sockAddr, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_port = htons(atoi(argv[1]));
        sockAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (sockaddr*)&sockAddr, sizeof (sockAddr)) == -1) {
            std::cerr << "bind() error";

            return 0;
        }

        if (listen(sock, 5) == -1) {
            std::cerr << "listen() error";
        
            return 0;
        }

    while (true) {
        //auto [clnt, clntAddr] {ca.accept(false)};
        int clnt = accept(sock, nullptr, nullptr);

        int value {1};
        setsockopt(clnt, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

        int flag {fcntl(clnt, F_GETFL)};
        flag |= O_NONBLOCK;
        fcntl(clnt, F_SETFL, flag);

        std::cout << "clnt " << clnt << " is working!\n";

        httpWorkder(clnt).run();
        
        std::cout << "clnt " << clnt << " is finished!\n";
    }  

    return 0;
}