#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <chrono>

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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./client <filename>\n";
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
        std::cerr << "inet_pton failed\n";
        close(sock);
        return 1;
    }

    if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    std::string filename = argv[1];
    uint32_t name_len = htonl((uint32_t)filename.size());

    if (!send_all(sock, &name_len, sizeof(name_len))) {
        std::cerr << "send name_len failed\n";
        close(sock);
        return 1;
    }

    if (!send_all(sock, filename.c_str(), filename.size())) {
        std::cerr << "send filename failed\n";
        close(sock);
        return 1;
    }

    uint32_t file_size_net;
    if (!recv_all(sock, &file_size_net, sizeof(file_size_net))) {
        std::cerr << "recv file_size failed\n";
        close(sock);
        return 1;
    }
    uint32_t file_size = ntohl(file_size_net);

    if (file_size == 0) {
        std::cout << "File not found on server.\n";
        close(sock);
        return 1;
    }

    uint32_t hash_len_net;
    if (!recv_all(sock, &hash_len_net, sizeof(hash_len_net))) {
        std::cerr << "recv hash_len failed\n";
        close(sock);
        return 1;
    }
    uint32_t hash_len = ntohl(hash_len_net);

    if (hash_len > 200) {
        std::cerr << "hash length too big\n";
        close(sock);
        return 1;
    }

    std::string server_hash(hash_len, '\0');
    if (!recv_all(sock, &server_hash[0], hash_len)) {
        std::cerr << "recv hash failed\n";
        close(sock);
        return 1;
    }

    std::string outname = "received_" + filename;
    std::ofstream output(outname, std::ios::binary);
    if (!output) {
        std::cerr << "could not create output file\n";
        close(sock);
        return 1;
    }

    auto start = std::chrono::steady_clock::now();

    char buffer[BUFFER_SIZE];
    uint32_t received = 0;

    while (received < file_size) {
        ssize_t bytes = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            std::cerr << "recv file data failed\n";
            output.close();
            close(sock);
            return 1;
        }
        output.write(buffer, bytes);
        received += (uint32_t)bytes;
    }

    auto end = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();

    output.close();
    close(sock);

    std::string client_hash = sha256_file(outname);

    std::cout << "File received successfully.\n";
    std::cout << "Server SHA256: " << server_hash << "\n";
    std::cout << "Client SHA256: " << client_hash << "\n";

    if (server_hash == client_hash) {
        std::cout << "Checksum: PASS\n";
    } else {
        std::cout << "Checksum: FAIL\n";
    }

    std::cout << "Transfer time: " << seconds << " seconds\n";

    double mb = file_size / (1024.0 * 1024.0);
    if (seconds > 0) {
        std::cout << "Speed: " << (mb / seconds) << " MB per second\n";
    }

    return 0;
}