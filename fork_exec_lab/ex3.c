#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid1 = fork();
    pid_t pid2 = fork();

    if (pid1 == 0) {
        printf("Child 1, PID: %d\n", getpid());
    } else if (pid2 == 0) {
        printf("Child 2, PID: %d\n", getpid());
    } else {
        printf("I am the parent, PID: %d\n", getpid());
    }

    return 0;
}
