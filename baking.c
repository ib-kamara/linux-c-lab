#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

int numBatterInBowl = 0;
int numEggInBowl = 0;
bool readyToEat = false;
pthread_mutex_t lock;
pthread_cond_t needIngredients, readyToBake, startEating;

void addBatter() {
    numBatterInBowl += 1;
    printf("Added batter. Batter = %d, Eggs = %d\n", numBatterInBowl, numEggInBowl);
}

void addEgg() {
    numEggInBowl += 1;
    printf("Added egg. Batter = %d, Eggs = %d\n", numBatterInBowl, numEggInBowl);
}

void heatBowl() {
    readyToEat = true;
    numBatterInBowl = 0;
    numEggInBowl = 0;
    printf("Cake is baked and ready to eat.\n");
}

void eatCake() {
    readyToEat = false;
    printf("Cake was eaten.\n");
}

void* batterAdder(void* arg) {
    pthread_mutex_lock(&lock);
    while (1) {
        while (numBatterInBowl == 1 || readyToEat) {
            pthread_cond_wait(&needIngredients, &lock);
        }
        addBatter();
        pthread_cond_signal(&readyToBake);
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

void* eggBreaker(void* arg) {
    pthread_mutex_lock(&lock);
    while (1) {
        while (numEggInBowl >= 2 || readyToEat) {
            pthread_cond_wait(&needIngredients, &lock);
        }
        addEgg();
        pthread_cond_signal(&readyToBake);
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

void* bowlHeater(void* arg) {
    pthread_mutex_lock(&lock);
    while (1) {
        while (numBatterInBowl < 1 || numEggInBowl < 2 || readyToEat) {
            pthread_cond_wait(&readyToBake, &lock);
        }
        heatBowl();
        pthread_cond_signal(&startEating);
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

void* cakeEater(void* arg) {
    pthread_mutex_lock(&lock);
    while (1) {
        while (!readyToEat) {
            pthread_cond_wait(&startEating, &lock);
        }
        eatCake();
        pthread_cond_broadcast(&needIngredients);
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&needIngredients, NULL);
    pthread_cond_init(&readyToBake, NULL);
    pthread_cond_init(&startEating, NULL);

    pthread_t batter, egg1, egg2, heater, eater;

    pthread_create(&batter, NULL, batterAdder, NULL);
    pthread_create(&egg1, NULL, eggBreaker, NULL);
    pthread_create(&egg2, NULL, eggBreaker, NULL);
    pthread_create(&heater, NULL, bowlHeater, NULL);
    pthread_create(&eater, NULL, cakeEater, NULL);

    while (1) {
        sleep(1);
    }

    return 0;
}
