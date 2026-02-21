#include <stdio.h>
#include <pthread.h>
#include <sched.h>

void* helper(void* arg) {
    printf("HELPER\n");
    return NULL;
}

int main() {
    pthread_t thread;

    pthread_create(&thread, NULL, &helper, NULL);

    pthread_join(thread, NULL);

    printf("MAIN\n");
    return 0;
}