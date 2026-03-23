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
#include <fstream>
#include <sstream>

std::queue<int> task_queue;
std::mutex queue_mutex;
std::condition_variable cv;
bool stop_pool = false;
const int NUM_THREADS = 10;

std::string get_content_type(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") return "text/css";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png") return "image/png";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".txt") return "text/plain";
    return "application/octet-stream";
}

std::string parse_request(int client_fd) {
    char buffer[4096];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

    if (bytes_read <= 0) {
        return "";
    }

    buffer[bytes_read] = '\0';
    std::string request(buffer, bytes_read);

    size_t pos = request.find('\n');
    if (pos == std::string::npos) {
        return "";
    }

    std::string first_line = request.substr(0, pos);

    size_t method_end = first_line.find(' ');
    if (method_end == std::string::npos || first_line.substr(0, 3) != "GET") {
        return "400 Bad Request";
    }

    size_t path_end = first_line.find(' ', method_end + 1);
    if (path_end == std::string::npos) {
        return "400 Bad Request";
    }

    std::string path = first_line.substr(method_end + 1, path_end - method_end - 1);

    size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        path = path.substr(0, query_pos);
    }

    if (path.empty() || path == "/") {
        path = "/index.html";
    }

    return path;
}

std::string get_file_content(const std::string& path) {
    if (path.find("..") != std::string::npos) {
        return "";
    }

    std::string fs_path = "." + std::string("/www") + path;

    std::ifstream file(fs_path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void send_response(int client_fd, const std::string& content, const std::string& content_type) {
    std::string status = content.empty() ? "404 Not Found" : "200 OK";

    std::string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += content;

    write(client_fd, response.c_str(), response.length());
}

void send_bad_request(int client_fd) {
    std::string body = "<h1>400 Bad Request</h1>";
    std::string response = "HTTP/1.1 400 Bad Request\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += body;

    write(client_fd, response.c_str(), response.length());
}

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
        }

        std::string path = parse_request(client_fd);

        if (path == "400 Bad Request") {
            send_bad_request(client_fd);
            close(client_fd);
            continue;
        }

        std::string content_type = get_content_type(path);
        std::string content = get_file_content(path);

        send_response(client_fd, content, content_type);
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

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return 1;
    }

    std::vector<std::thread> workers;
    for (int i = 0; i < NUM_THREADS; ++i) {
        workers.emplace_back(worker_thread);
    }

    std::cout << "HTTP server running on port " << port << std::endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        std::cout << "HTTP client connected from "
                  << inet_ntoa(client_addr.sin_addr) << std::endl;

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            task_queue.push(client_fd);
        }

        cv.notify_one();
    }

    close(server_fd);
    return 0;
}