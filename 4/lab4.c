/* Elijah Morris
 * 9-14-21
 * Lab 4
 */
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>


int main() {
    pid_t child;
    key_t ipckey;
    int logfd;
    int shmid;
    int *shared;
    int status;
    int mqid;
    char pathname[128], buf[4], bufToStr[4];

    struct {
        char text[100];
    } mymsg;
    
    int n = 0;

    getcwd(pathname, 128);
    strcat(pathname, "/foo");
    
    ipckey = ftok(pathname, 21);

    logfd = open("log", O_WRONLY|O_CREAT|O_TRUNC, 0644);

    shmid = shmget(ipckey, sizeof(int), IPC_CREAT | 0666);
    shared = shmat(shmid, (void *) 0, 0);

    mqid = msgget(ipckey, IPC_CREAT | 0666);

    child = fork();

    if(child == 0) {
        int received;

        while(*shared <= 0) {}

        sprintf(bufToStr, "%i\n", *shared);
        write(logfd, bufToStr, strlen(bufToStr));
        received = msgrcv(mqid, &mymsg, sizeof(mymsg), 0, 0);
        if(received > 0) {
            write(logfd, mymsg.text, strlen(mymsg.text));
        }
        msgctl(mqid, IPC_RMID, NULL);
        close(logfd);
        shmdt(shared);
        exit(n);

    }
    else {
        memset(buf, 0, strlen(buf));
        write(1, "Enter a number from 10-99: ", 27);
        read(0, buf, 4);
        *shared = atoi(buf);

        memset(mymsg.text, 0, strlen(mymsg.text));
        write(1, "Enter a word: ", 15);
        read(0, mymsg.text, 100);
        
        msgsnd(mqid, &mymsg, sizeof(mymsg), 0);

        wait(&status);
        printf("child exited with code: %i\n", n);
        shmdt(shared);


    }

    return 0;
}





