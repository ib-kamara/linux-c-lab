#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    signal(SIGINT, SIG_IGN);
    printf("Running. PID=%d\n", getpid());

    while (1) {
        sleep(1);
    }
}
