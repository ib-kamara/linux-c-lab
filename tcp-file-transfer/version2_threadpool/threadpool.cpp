#include "threadpool.h"
#include <unistd.h>
#include <iostream>

extern void send_file(int client_socket);

ThreadPool::ThreadPool(size_t threads) : stop(false) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back(&ThreadPool::worker, this);
    }
}

void ThreadPool::enqueue(int client_socket) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(client_socket);
    }
    condition.notify_one();
}

void ThreadPool::worker() {
    while (true) {
        int client_socket;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] {
                return stop || !tasks.empty();
            });

            if (stop && tasks.empty()) return;

            client_socket = tasks.front();
            tasks.pop();
        }

        send_file(client_socket);
        close(client_socket);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();

    for (auto& t : workers) {
        t.join();
    }
}