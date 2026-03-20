#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t lock;
pthread_cond_t reader_cv, writer_cv;
int reader_count = 0, writer_waiting = 0;
int shared_data = 0;

void* reader(void* arg) {
    pthread_mutex_lock(&lock);
    while (writer_waiting > 0) {
        pthread_cond_wait(&reader_cv, &lock);
    }
    reader_count++;
    pthread_mutex_unlock(&lock);

    printf("Reader reads: %d\n", shared_data);

    pthread_mutex_lock(&lock);
    reader_count--;
    if (reader_count == 0) {
        pthread_cond_signal(&writer_cv);
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

void* writer(void* arg) {
    pthread_mutex_lock(&lock);
    writer_waiting++;
    while (reader_count > 0) {
        pthread_cond_wait(&writer_cv, &lock);
    }

    shared_data++;
    printf("Writer writes: %d\n", shared_data);

    writer_waiting--;
    pthread_cond_broadcast(&reader_cv);
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t r1, r2, r3, w1, w2;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&reader_cv, NULL);
    pthread_cond_init(&writer_cv, NULL);

    pthread_create(&r1, NULL, reader, NULL);
    pthread_create(&r2, NULL, reader, NULL);
    pthread_create(&w1, NULL, writer, NULL);
    pthread_create(&r3, NULL, reader, NULL);
    pthread_create(&w2, NULL, writer, NULL);

    pthread_join(r1, NULL);
    pthread_join(r2, NULL);
    pthread_join(r3, NULL);
    pthread_join(w1, NULL);
    pthread_join(w2, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&reader_cv);
    pthread_cond_destroy(&writer_cv);

    return 0;
}