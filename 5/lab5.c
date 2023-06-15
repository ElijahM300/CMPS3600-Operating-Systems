/* Elijah Morris
 * lab5.c
 *
 * You may start with this program for lab-5
 * 
 * This file uses shared memory. If you do not understand how shared memory 
 * works, review week-4 examples. Your job in lab-5 is to add a semaphore 
 * to control order. You know things have gone awry when you run the code:
 *  
 *    $ make lab5
 *    $ ./lab5
 *    child reads: 0
 *
 * The desired result is for the parent to compute fib(n), write the result to
 * shared memory then the child reads the result and displays it to the screen.
 * The problem is that things are out of order - by the time the parent computes
 * fib(10) the child has already read memory; i.e., the parent modifies the 
 * segment too late.
 * 
 * This scenario is a race condition. For example, if you pass a small enough 
 * number to fib, the child generally grabs the value OK:
 *
 *    $ ./lab5 10
 *    child reads: 55 
 *
 * but this may not work
 *
 *    $ ./lab5 18
 *    child reads: 0  <---- wrong results 
 *
 * To fix this problem you need to add a semaphore to control order. You want 
 * the parent to grab the semaphore first. Meanwhile the child is blocked on 
 * the semaphore. After the parent writes fib(n) to memory the parent releases 
 * the semaphore and the child can then grab it.
 * 
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/sem.h>


int status;
pid_t cpid;
#define BUFSIZE 256

int fib(int);
void sigint_handler(int sig);

union {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} my_semun;

struct sembuf grab[2], release[1];
int n_waiting, sem_value;
pid_t lastpid;
char pathname[200];
key_t ipckey;
int semid;

struct sigaction sa;

int main(int argc, char **argv)
{
	int n;
	char buf[BUFSIZE];
	int shmid; 
	int *shared;
    int logfd;

	/* check if n was passed */
	if (argc >= 2)
		n = atoi(argv[1]);
	else
		n = 20;

    shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    shared = shmat(shmid, (void *) 0, 0);
    *shared = 0;

    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigfillset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

	getcwd(pathname, 200);
    strcat(pathname, "/foo");
    ipckey = ftok(pathname, 42); 
    int nsem = 1;
    semid = semget(ipckey, nsem, 0666 | IPC_CREAT);
    if(semid < 0) {
        printf("Error - %s\n", strerror(errno));
        _exit(1);
    }

    my_semun.val = 0;
    semctl(semid, 0, SETVAL, my_semun);
    sem_value = semctl(semid, 0, GETVAL);

    grab[0].sem_num = 0;
    grab[1].sem_num = 0;

    grab[0].sem_flg = SEM_UNDO;
    grab[1].sem_flg = SEM_UNDO;
    
    logfd = open("log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    printf("starting fork...\n");
    cpid = fork();

    grab[0].sem_op = 0;
    grab[1].sem_op = +1;
    semop(semid, grab, 2);

    n_waiting = semctl(semid, 0, GETZCNT);
    if(n_waiting) {
        printf("%d process blocking on zero\n", n_waiting);
    }    

    n_waiting = semctl(semid, 0, GETNCNT);
    if(n_waiting) {
        printf("waiting for pos value: %d\n", n_waiting);
    }

    release[0].sem_num = 0;
    release[0].sem_flg = SEM_UNDO;
    release[0].sem_op = -1;

	if (cpid != 0) {
        printf("parent process\n");
		/* PARENT */
        sleep(1);
		/* parent computes fib(n) and writes it to shared memory */
		/* grab a semaphore */
		*shared = fib(n);
		/* release the child's semaphore */
        printf("parent releasing the semaphore...\n");
        semop(semid, release, 1);
		wait(&status); 
		/* detach from segment */
        shmdt(shared);
        if((semctl(semid, 0, IPC_RMID) < 0)) {
            perror("semctrl IPC_RMID");
            exit(EXIT_FAILURE);
        }
	} else {
        printf("child process\n");
	    /* CHILD */
		/* Attach to shared memory -
		 * both child and parent must do this
		 * but the parent can do it before the fork */
        logfd = open("log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
		shared = shmat(shmid, (void *) 0, 0);
		/* child reads and displays shared memory */
		int val = *shared;
        sprintf(buf, "child reads: %d\n", val);
        puts(buf);
		write(logfd, buf, strlen(buf));
		/* detach from segment */
		shmdt(shared);
        shmctl(shmid, IPC_RMID, 0);
        close(logfd);
		exit(0);
	}
	return 0;
}

/* Some busy work for the parent */
int fib(int n)
{
	/* We will write this function in class together. */
	if (n == 1)
		return 1;
	if (n == 2)
		return 1;
	return fib(n-1) + fib(n-2);
}

void sigint_handler(int sig) {
    char buf[50];
    if(cpid == 0) {
        _exit(77);
    } 
    else {
        strcpy(buf, "Ctrl-c received and handled by parent\n");
        write(1, buf, strlen(buf));
        if(semctl(semid, 0, IPC_RMID) < 0) {
            perror("IPC_RMID error: ");
        }
        else
            write(1, "semaphore removed\n", 19);
        _exit(0);
    }
}







