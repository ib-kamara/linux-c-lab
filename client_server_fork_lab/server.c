#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 8080

void handle_client(int client_sock) {
    char buffer[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int n = read(client_sock, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            break;
        }

        buffer[strcspn(buffer, "\r\n")] = 0;

        printf("Received: %s\n", buffer);

        if (strcmp(buffer, "exit") == 0) {
            write(client_sock, "bye", 3);
            break;
        }

        char reply[1100];
        snprintf(reply, sizeof(reply), "Echo: %s", buffer);
        write(client_sock, reply, strlen(reply));
    }

    close(client_sock);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        if (client_sock < 0) {
            perror("accept failed");
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            close(server_sock);
            handle_client(client_sock);
            exit(0);
        }
        else if (pid > 0) {
            close(client_sock);
            while (waitpid(-1, NULL, WNOHANG) > 0) {}
        }
        else {
            perror("fork failed");
        }
    }

    return 0;
}