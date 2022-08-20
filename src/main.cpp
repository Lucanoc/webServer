#include <iostream>

#include "threadPool.hpp"
#include "epollWorker.hpp"

auto main() -> int {
    connAccepter accepter(9190);

    epollWorker(accepter).wait();

    return 0;
}