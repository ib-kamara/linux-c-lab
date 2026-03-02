
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include "threadpool.h"

#define PORT 9000
#define BUFFER_SIZE 4096
#define THREADS 4

static bool recv_all(int sock, void* data, size_t len) {
    char* p = (char*)data;
    size_t total = 0;
    while (total < len) {
        ssize_t n = recv(sock, p + total, len - total, 0);
        if (n <= 0) return false;
        total += (size_t)n;
    }
    return true;
}

static bool send_all(int sock, const void* data, size_t len) {
    const char* p = (const char*)data;
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(sock, p + total, len - total, 0);
        if (n <= 0) return false;
        total += (size_t)n;
    }
    return true;
}

static std::string sha256_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return "";

    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    char buf[4096];
    while (file) {
        file.read(buf, sizeof(buf));
        std::streamsize n = file.gcount();
        if (n > 0) SHA256_Update(&ctx, buf, (size_t)n);
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &ctx);

    std::ostringstream out;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        out << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return out.str();
}

void send_file(int client_socket) {
    uint32_t name_len;

    if (!recv_all(client_socket, &name_len, sizeof(name_len))) return;
    name_len = ntohl(name_len);

    if (name_len >= 256) {
        uint32_t size0 = htonl(0);
        send_all(client_socket, &size0, sizeof(size0));
        return;
    }

    char filename[256] = {0};
    if (!recv_all(client_socket, filename, name_len)) return;

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        uint32_t size0 = htonl(0);
        send_all(client_socket, &size0, sizeof(size0));
        return;
    }

    file.seekg(0, std::ios::end);
    uint32_t file_size = (uint32_t)file.tellg();
    file.seekg(0);

    std::string hash_hex = sha256_file(filename);

    uint32_t net_size = htonl(file_size);
    send_all(client_socket, &net_size, sizeof(net_size));

    uint32_t hash_len = (uint32_t)hash_hex.size();
    uint32_t net_hash_len = htonl(hash_len);
    send_all(client_socket, &net_hash_len, sizeof(net_hash_len));
    send_all(client_socket, hash_hex.c_str(), hash_hex.size());

    char buffer[BUFFER_SIZE];
    while (file) {
        file.read(buffer, BUFFER_SIZE);
        std::streamsize bytes = file.gcount();
        if (bytes > 0)
            send_all(client_socket, buffer, (size_t)bytes);
    }

    file.close();
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return 1;
    }

    ThreadPool pool(THREADS);

    std::cout << "Thread pool server running with "
              << THREADS << " threads\n";

    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        pool.enqueue(client_socket);
    }

    close(server_fd);
    return 0;
}