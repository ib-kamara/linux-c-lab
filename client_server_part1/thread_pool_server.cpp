#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>

std::queue<int> task_queue;
std::mutex queue_mutex;
std::condition_variable cv;
bool stop_pool = false;
const int NUM_THREADS = 3;

void worker_thread() {
    while (true) {
        int client_fd;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [] { return !task_queue.empty() || stop_pool; });

            if (stop_pool && task_queue.empty()) {
                return;
            }

            client_fd = task_queue.front();
            task_queue.pop();

            std::cout << "Worker picked a client. Queue size now: "
                      << task_queue.size() << std::endl;
        }

        char buffer[1024];
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "Received: " << buffer << std::endl;
            write(client_fd, buffer, bytes_read);
        }

        close(client_fd);
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;

    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        return 1;
    }

    std::vector<std::thread> workers;
    for (int i = 0; i < NUM_THREADS; i++) {
        workers.emplace_back(worker_thread);
    }

    std::cout << "Thread-pool server running on port " << port << std::endl;
    std::cout << "Number of worker threads: " << NUM_THREADS << std::endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        std::cout << "Client queued from "
                  << inet_ntoa(client_addr.sin_addr) << std::endl;

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            task_queue.push(client_fd);
            std::cout << "Queue size after push: "
                      << task_queue.size() << std::endl;
        }

        cv.notify_one();
    }

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop_pool = true;
    }

    cv.notify_all();

    for (auto& w : workers) {
        w.join();
    }

    close(server_fd);
    return 0;
}