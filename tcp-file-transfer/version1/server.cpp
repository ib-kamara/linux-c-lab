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

#define PORT 9000
#define BUFFER_SIZE 4096

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

    if (!recv_all(client_socket, &name_len, sizeof(name_len))) {
        std::cerr << "recv name_len failed\n";
        return;
    }
    name_len = ntohl(name_len);

    if (name_len >= 256) {
        std::cerr << "filename too long\n";
        uint32_t size0 = htonl(0);
        send_all(client_socket, &size0, sizeof(size0));
        return;
    }

    char filename[256] = {0};
    if (!recv_all(client_socket, filename, name_len)) {
        std::cerr << "recv filename failed\n";
        return;
    }

    std::cout << "Requested file: " << filename << std::endl;

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "File not found on server\n";
        uint32_t size0 = htonl(0);
        send_all(client_socket, &size0, sizeof(size0));
        return;
    }

    file.seekg(0, std::ios::end);
    uint32_t file_size = (uint32_t)file.tellg();
    file.seekg(0);

    std::cout << "File size: " << file_size << " bytes" << std::endl;

    std::string hash_hex = sha256_file(filename);
    std::cout << "SHA256: " << hash_hex << std::endl;

    uint32_t net_size = htonl(file_size);
    if (!send_all(client_socket, &net_size, sizeof(net_size))) {
        std::cerr << "send file size failed\n";
        return;
    }

    uint32_t hash_len = (uint32_t)hash_hex.size(); // should be 64
    uint32_t net_hash_len = htonl(hash_len);

    if (!send_all(client_socket, &net_hash_len, sizeof(net_hash_len))) {
        std::cerr << "send hash_len failed\n";
        return;
    }

    if (!send_all(client_socket, hash_hex.c_str(), hash_hex.size())) {
        std::cerr << "send hash failed\n";
        return;
    }

    char buffer[BUFFER_SIZE];
    while (file) {
        file.read(buffer, BUFFER_SIZE);
        std::streamsize bytes = file.gcount();
        if (bytes > 0) {
            if (!send_all(client_socket, buffer, (size_t)bytes)) {
                std::cerr << "send file data failed\n";
                return;
            }
        }
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
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ipstr, sizeof(ipstr));
        std::cout << "Client IP: " << ipstr << std::endl;

        send_file(client_socket);
        close(client_socket);
    }

    close(server_fd);
    return 0;
}