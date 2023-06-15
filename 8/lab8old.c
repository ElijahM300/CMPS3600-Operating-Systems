//Elijah Morris
//10-12-21
//Lab 8
//phil8 - Dining philosophers program
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
//#include <sys/files.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semid;
struct sembuf forks_ops[6][2];
struct sembuf release[6][1];
int eat[5] = {0,0,0,0,0};

union {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} my_semun;

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
        forks_ops[5][0].sem_op = 0;
        forks_ops[5][1].sem_op = +1;
        semop(semid, forks_ops[5], 2);
        //pick up a fork
        semop(semid, forks_ops[myforks[0]], 2);
        //
        //pick up another fork
        printf("number: %i pick up fork %i\n", number, myforks[1]);
        semop(semid, forks_ops[myforks[1]], 2);

        release[5][0].sem_op = -1;
        semop(semid, release[5], 1);
        //eat
        ++eat[number];
        printf("eat: %i %i %i %i %i\n",eat[0],eat[1],eat[2],eat[3],eat[4]);
        //
        //put down a fork
        printf("number: %i put down fork %i\n", number, myforks[1]);
        for(int i = 0; i < 5; i++) {
            release[i][0].sem_op = -1;
        }
        semop(semid, release[myforks[1]], 1);
        //put down another fork
        printf("number: %i put down fork %i\n", number, myforks[0]);
        for(int i = 0; i < 5; i++) {
            release[i][1].sem_op = -1;
        }
        semop(semid, release[myforks[0]], 1);
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
    int nsems = 6;
    void *status;

    semid = semget(IPC_PRIVATE, nsems, 0666 | IPC_CREAT);
    if(semid < 0) {
        perror("semget: ");
        _exit(1);
    }

    my_semun.val = 0;
    semctl(semid, 0, SETVAL, my_semun);
    my_semun.val = 1;
    semctl(semid, 1, SETVAL, my_semun);
    my_semun.val = 2;
    semctl(semid, 2, SETVAL, my_semun);
    my_semun.val = 3;
    semctl(semid, 3, SETVAL, my_semun);
    my_semun.val = 4;
    semctl(semid, 4, SETVAL, my_semun);
    my_semun.val = 5;
    semctl(semid, 5, SETVAL, my_semun);

    for(int i = 0; i < 5; i++) {
        forks_ops[i][0].sem_num = i;
        forks_ops[i][0].sem_op = 0;
        forks_ops[i][0].sem_flg = SEM_UNDO;

        forks_ops[i][1].sem_num = i;
        forks_ops[i][1].sem_op = +1;
        forks_ops[i][1].sem_flg = SEM_UNDO;

        release[i][0].sem_num = i;
        release[i][0].sem_op = -1;
        release[i][0].sem_flg = SEM_UNDO;
    }

    forks_ops[5][0].sem_num = 5;
    forks_ops[5][0].sem_op = 0;
    forks_ops[5][0].sem_flg = SEM_UNDO;

    forks_ops[5][1].sem_num = 5;
    forks_ops[5][1].sem_op = +1;
    forks_ops[5][1].sem_flg = SEM_UNDO;

    release[5][0].sem_num = 5;
    release[5][0].sem_op = -1;
    release[5][0].sem_flg = SEM_UNDO;

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
