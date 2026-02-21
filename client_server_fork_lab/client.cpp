#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024] = "Hello Server";
    char recvbuf[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    write(sock, buffer, strlen(buffer));
    memset(recvbuf, 0, sizeof(recvbuf));
    read(sock, recvbuf, sizeof(recvbuf) - 1);

    printf("Server response: %s\n", recvbuf);

    close(sock);
    return 0;
}