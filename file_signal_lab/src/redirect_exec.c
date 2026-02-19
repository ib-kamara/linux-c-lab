#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s output_file\n", argv[0]);
        exit(1);
    }

    int newfd = open(argv[1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
    if (newfd < 0) {
        perror("open");
        exit(1);
    }

    printf("Redirecting ls output into %s\n", argv[1]);

    dup2(newfd, 1);

    char *cmd[] = {"/bin/ls", "-l", NULL};
    execvp(cmd[0], cmd);

    perror("execvp failed");
    exit(1);
}
