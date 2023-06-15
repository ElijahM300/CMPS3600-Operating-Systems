/*  Elijah Morris
 *  Lab 6
 *  9-28-21
 *
 *  lab6.c
 *
 *  Demonstrate classic producer/consumer problem; the parent thread (the 
 *  producer) writes one character at a time from a file into a shared buffer
 *
 *  The consumer reads from buffer and writes it to a log file. With no 
 *  synchronization, the consumer will read the same character multiple times
 *  because the producer is slower than the consumer.
 *  Output is not deterministic - it varies across executions. 
 *
 *  This is a problem of concurrency. When two threads access shared resources 
 *  without synchronization it is a race condition as to which thread gets 
 *  there first or next. Without synchronization, the consumer may read a value
 *  multiple times (empty buffer) or the producer overwrite a value before the 
 *  consumer had a chance to read it. 
 *
 *    $ gcc -o lab6 lab6.c -lpthread   # link in POSIX pthread library  
 *    $ ./lab6
 */

#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/sem.h>
#include <fcntl.h>

#define DEBUG 0 

/* thread function prototypes */
void consumer(void *dummy1); 
void producer(void *dummy2);

/* A note on the differences between forks and threads. Variables local to 
 * main that exist before the fork are inherited by the child but not shared.
 * Threads, since they are functions, can only see globals. These globals are
 * not only visible but shared by all threads since threads share user space.
 */

int semid;
struct sembuf pro_sops[1];
struct sembuf con_sops[1];
int retval;
int LIMIT = 100;        /* for testing read 50 chars from the file */
char buf[1];           /* 1 char buffer */
int fib(int);
FILE * outf;
FILE * inf;

union {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} my_semun;

int main(int argc, char *argv[])
{
	pthread_t cthr, pthr;  /* cthr will point to the consumer thread */
	int dummy1, dummy2;
    int nsems = 2;

	/* we are using formatted fopen(2) to make life easier - normally
	 * in systems coding you would use open(2)
	 */
	inf = fopen("poem", "r");
	if (!inf) {
		fprintf(stderr, "error opening input file.\n");
		exit(1);
	}

    if(argc == 2) {
        LIMIT = atoi(argv[1]);
    }

    outf = fopen("log", "w");
    if (outf == NULL) {
        fprintf(stderr, "error opening output file.\n");
        exit(1);
    } 

    semid = semget(IPC_PRIVATE, nsems, 0666 | IPC_CREAT);
    if(semid < 0) {
        perror("semget: ");
        _exit(1);
    }

    my_semun.val = 1;
    semctl(semid, 0, SETVAL, my_semun);
    my_semun.val = 0;
    semctl(semid, 1, SETVAL, my_semun);

    pro_sops[0].sem_num = 0;
    pro_sops[0].sem_flg = SEM_UNDO;

    con_sops[0].sem_num = 1;
    con_sops[0].sem_flg = SEM_UNDO;

	/* create consumer thread */
	if (pthread_create(&cthr, NULL,  (void *)consumer, (void *)&dummy1) != 0)
		fprintf(stderr,"Error creating thread\n");

	buf[0] = ' ';
	pid_t tid = syscall(SYS_gettid);
	if (DEBUG)
		printf("main thread pid: %d tid: %d \n",getpid(), tid);

	/* create producer thread */
    if (pthread_create(&pthr, NULL, (void *)producer, (void *)&dummy2) != 0) {
        fprintf(stderr, "Error creating thread\n");
    }

	/* the parent thread always joins with its spawned threads */
	if ((pthread_join(cthr, (void*)&retval)) < 0 || (pthread_join(pthr, (void*)&retval)) < 0) 
		perror("pthread_join");
	else  
		if (DEBUG) printf("joined consumer thread w exit code: %d\n",retval);

	/* parent closes input file */
	fclose(inf);
    /* parent closes output file */ 
    fclose(outf);
	exit(0);
}

/* CONSUMER thread function 
 * reads 1 char from shared buffer and writes char to screen 
 */
void consumer(void *dummy1)
{
	int count = 0;
	pid_t tid = syscall(SYS_gettid);      
	fprintf(stdout,"consumer thread pid: %d tid: %d \n",getpid(), tid);
	while (1) {
		if (count == LIMIT) 
			break;
        /* grab consumer sem */
        con_sops[0].sem_op = -1;
        semop(semid, con_sops, 1);
		fputc(buf[0],stdout);
		fib(15);   /* make consumer slightly faster than producer */
		count++; 
        /* release producer sem */
        pro_sops[0].sem_op = +1;
        semop(semid, pro_sops, 1);
	}
	fputc('\n',stdout);
	pthread_exit(0);
}

/* PRODUCER thread function 
 * reads 1 char from poem file and writes the char to buffer
 */
void producer(void *dummy2) 
{
    int count = 0;
    while(1) {
        if(count == LIMIT)
            break;
        /* grab producer sem */
        pro_sops[0].sem_op = -1;
        semop(semid, pro_sops, 1);
        /* critical code */
        buf[0] = fgetc(inf);
        if(DEBUG)
            putc(buf[0], stdout);
        fib(16);    /* make the producer slightly slower than the consumer */
        count++;
        /* release consumer sem */
        con_sops[0].sem_op = +1;
        semop(semid, con_sops, 1);
    }

}

int fib(int n)
{
	if (n == 1)
		return 1;
	if (n == 2)
		return 1;
	return fib(n-1) + fib(n-2);
}
