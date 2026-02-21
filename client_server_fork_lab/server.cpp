#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define PORT 8080

void handle_client(int client_sock) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int n = read(client_sock, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        std::cout << "Received: " << buffer << std::endl;
        write(client_sock, "Hello from C++ server", 21);
    }

    close(client_sock);
}

int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
int *client_count = (int*) shmat(shmid, NULL, 0);
*client_count = 0;
    int server_sock, client_sock;
    sockaddr_in server_addr{}, client_addr{};
    socklen_t addr_size;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_sock, 5) < 0) {
        perror("listen");
        return 1;
    }

    std::cout << "C++ Server listening on port " << PORT << std::endl;

    while (true) {
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (sockaddr*)&client_addr, &addr_size);
pid_t pid = fork();

if (pid == 0) {
    close(server_sock);

    (*client_count)++;
    std::cout << "Active clients: " << *client_count << std::endl;
sleep(5);
    handle_client(client_sock);

    (*client_count)--;
    std::cout << "Active clients: " << *client_count << std::endl;

    close(client_sock);
    return 0;
}
else if (pid > 0) {
    close(client_sock);
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}
else {
    perror("fork");
}
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

       

        if (pid == 0) {
            close(server_sock);
            handle_client(client_sock);
            return 0;
        } else if (pid > 0) {
            close(client_sock);
            while (waitpid(-1, NULL, WNOHANG) > 0) {}
        } else {
            perror("fork");
        }
    }

    return 0;
}
