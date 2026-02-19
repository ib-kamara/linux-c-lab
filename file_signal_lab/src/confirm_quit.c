#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

void handler(int sig) {
    char answer;

    write(1, "\nDo you really want to quit [y/n]? ", 35);
    read(0, &answer, 1);

    if (answer == 'y') {
        write(1, "\nExiting...\n", 12);
        exit(0);
    } else {
        write(1, "\nContinuing...\n", 15);
    }
}

int main() {
    signal(SIGINT, handler);

    while (1) {
        sleep(1);
    }
}
