//Elijah Morris
//10-12-21
//Lab 8
//phil8 - Dining philosophers program
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

sem_t forks[5];
sem_t monitor;
int eat[5] = {0,0,0,0,0};

void *philosopher(void *args) {
    int number = (int)(long)(args);
    int myforks[2] = {number, (number+1)%5};
    //sort the forks
    if(myforks[0] > myforks[1]) {
        //swap
        int tmp = myforks[0];
        myforks[0] = myforks[1];
        myforks[1] = tmp;
    }
    while(1){
        //thinking...
        printf("number: %i thinking\n", number);
        //
        sem_wait(&monitor);
        //pick up a fork
        printf("number: %i pick up fork %i\n", number, myforks[0]);
        sem_wait(&forks[myforks[0]]);
        //
        //pick up another fork
        printf("number: %i pick uo fork %i\n", number, myforks[1]);
        sem_wait(&forks[myforks[1]]);
        //
        sem_post(&monitor);
        //eat
        ++eat[number];
        printf("eat: %i %i %i %i %i\n",eat[0],eat[1],eat[2],eat[3],eat[4]);
        //
        //put down a fork
        printf("number: %i put down fork %i\n", number, myforks[1]);
        sem_post(&forks[myforks[1]]);
        //
        //put down another fork
        printf("number: %i put down fork %i\n", number, myforks[0]);
        sem_post(&forks[myforks[0]]);
        //
        if(eat[number] > 100000) {
            break;
        }
        //sleep
        //
    }
    return (void *)0;
}

int main(void) {
    int i,ret;
    void *status;
    for(i = 0; i < 5; i++) {
        sem_init(&forks[i], 0, 1);
    }
    sem_init(&monitor, 0, 1);
    pthread_t th[5];
    for(i = 0; i < 5; i++) {
        ret = pthread_create(&th[i], NULL, philosopher, (void *)(long)i);
        if(ret != 0) {
            printf("Error in pthread_create\n");
            exit(0);
         }
    }
    for(i = 0; i < 5; i++) {
        pthread_join(th[i], &status);
    }
    return 0;
}
