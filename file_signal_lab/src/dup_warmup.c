#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int newfd;

    if ((newfd = open("output_file.txt", O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
        perror("open");
        exit(1);
    }

    printf("Luke, I am your...\n");

    dup2(newfd, 1);

    printf("father\n");

    close(newfd);
    return 0;
}
