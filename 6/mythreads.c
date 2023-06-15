//
//
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *myThread(void *args) {
    int i;
    for(i = 0; i < 4; i++) {
        printf("thread number %ld is running\n", (long)args);
        //usleep(rand() % 200 + 20);
    }
    fflush(stdout);
    //pthread_exit(NULL);
    return (void *)0;
}

int main(void) {
    int ret;
    void *status;
    pthread_t th1, th2;
    ret = pthread_create(&th1, NULL, myThread, (void *)1);
    ret = pthread_create(&th2, NULL, myThread, (void *)2);
    if(ret != 0) {
        printf("Error in pthread_create\n");
        exit(0);
    }
    pthread_join(th1, &status);
    pthread_join(th2, &status);
    //pthread_exit(NULL);
    return 0;
}
