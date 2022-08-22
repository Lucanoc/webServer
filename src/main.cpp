#include <arpa/inet.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

#include "connAccepter.hpp"
#include "httpWorker.hpp"
#include "threadPool.hpp"
#include "epollWorker.hpp"

auto main(int argc, char ** argv) -> int {
    // if (argc < 3) {
    //     std::cerr << "usage: appname [port] [work directory]\n";

    //     return 0;
    // }

    // chdir(argv[2]);

    // int port {atoi(argv[1])};

    // epollWorker(connAccepter(port)).run([](int sockFd){
    //     httpWorker hs(sockFd);

    //     if (hs.recv() == httpWorker::workStatus::recvFinish) {
    //         hs.send();
    //     }
    // });

    std::cout << "Hello World!\n";

    return 0;
}