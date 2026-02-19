#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s output_file\n", argv[0]);
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // child: redirect stdout to file and exec ls
        int fd = open(argv[1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
        if (fd < 0) {
            perror("open");
            exit(1);
        }

        dup2(fd, 1);
        close(fd);

        char *cmd[] = {"/bin/ls", "-l", NULL};
        execvp(cmd[0], cmd);

        perror("execvp failed");
        exit(1);
    }

    // parent: wait for child to finish
    wait(NULL);
    printf("all done\n");
    return 0;
}
