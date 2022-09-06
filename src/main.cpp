#include "epollWorker.hpp"
#include "httpWorker.hpp"
#include "connAccepter.hpp"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <limits>
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

    // connAccepter ca(atoi(argv[1]));

    // while (true) {
    //     std::pair<int, sockaddr_in> sockInfo(ca.accept(false));

    //     httpWorkder(sockInfo.first).run();
    // }

    return 0;
}