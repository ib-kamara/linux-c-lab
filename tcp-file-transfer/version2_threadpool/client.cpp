#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define PORT 9000
#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./client <filename>\n";
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    connect(sock, (sockaddr*)&server, sizeof(server));

    std::string filename = argv[1];
    uint32_t name_len = htonl(filename.size());

    send(sock, &name_len, sizeof(name_len), 0);
    send(sock, filename.c_str(), filename.size(), 0);

    uint32_t file_size;
    recv(sock, &file_size, sizeof(file_size), 0);
    file_size = ntohl(file_size);

    if (file_size == 0) {
        std::cout << "File not found on server.\n";
        return 1;
    }

    std::ofstream output("received_" + filename, std::ios::binary);

    char buffer[BUFFER_SIZE];
    uint32_t received = 0;

    while (received < file_size) {
        int bytes = recv(sock, buffer, BUFFER_SIZE, 0);
        output.write(buffer, bytes);
        received += bytes;
    }

    output.close();
    close(sock);

    std::cout << "File received successfully.\n";
    return 0;
}