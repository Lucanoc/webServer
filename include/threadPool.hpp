#pragma once

#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <future>
#include <queue>
#include <utility>
#include <vector>
#include <functional>
#include <memory>

class threadPool {
public:
    ~threadPool();
    explicit threadPool(size_t threadsNum);

    template <typename Func, typename ... Args>
    auto submit(Func && func, Args && ... args) -> std::future<decltype(func(args...))>;

private:
    bool stop;
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> threads;
    std::mutex mut;
    std::condition_variable cond;
};

inline
threadPool::~threadPool() {
    {
        std::scoped_lock<std::mutex> lk(mut);

        stop = true;
    }

    cond.notify_all();

    for (std::thread & workder : threads) {
        workder.join();
    }
}

inline
threadPool::threadPool(size_t threadsNum) 
: stop(false) {
    for (size_t i {}; i != threadsNum; ++i) {
        threads.emplace_back([this]{
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lk(mut);

                    cond.wait(lk, [this]{return stop || !tasks.empty();});

                    if (stop && tasks.empty()) {
                        return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }

                task();
            }
        });
    }
}

template <typename Func, typename ... Args>
auto threadPool::submit(Func && func, Args && ... args) 
-> std::future<decltype(func(args...))> {
    auto taskPtr {std::make_shared<std::packaged_task<decltype(func(args...))()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
    )};

    {
        std::scoped_lock<std::mutex> lk(mut);

        if (stop) {
            throw std::runtime_error("submit on stopped threadPool.");
        }

        tasks.emplace([taskPtr]{(*taskPtr)();});
    }

    cond.notify_one();

    return taskPtr->get_future();
}

namespace concurrency {
    class threadPool {
    public:
    ~threadPool();
    threadPool(size_t threadsNum);

    template <typename FUNC, typename ... Args>
    auto submit(FUNC && func, Args && ... args) -> std::future<decltype(func(args...))>;

    private:
        bool stop;
        std::mutex mut;
        std::condition_variable cond;
        std::queue<std::function<void()>> tasks;
        std::vector<std::thread> workers;
    };

    inline
    threadPool::~threadPool() {
        {
            std::scoped_lock<std::mutex> lk(mut);

            stop = true;
        }

        cond.notify_all();

        for (std::thread & worker : workers) {
            worker.join();
        }
    }

    inline
    threadPool::threadPool(size_t threadsNum) 
    : stop(false) {
        for (size_t i {}; i != threadsNum; ++i) {
            workers.emplace_back([this]{
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lk(mut);

                        cond.wait(lk, [this]{return stop || !tasks.empty();});

                        if (stop && tasks.empty()) {
                            return;
                        }

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    template <typename FUNC, typename ... Args>
    auto threadPool::submit(FUNC && func, Args && ... args)
    -> std::future<decltype(func(args...))> {
        auto taskPtr {std::make_shared<std::packaged_task<decltype(func(args...))()>>(
            std::bind(std::forward<FUNC>(func), std::forward<Args>(args)...)
        )};

        {
            std::scoped_lock<std::mutex> lk(mut);

            if (stop) {
                throw std::runtime_error("submit on a stopped threadPool.");
            }

            tasks.emplace([taskPtr]{(*taskPtr)();});
        }

        cond.notify_one();

        return taskPtr->get_future();
    }
}