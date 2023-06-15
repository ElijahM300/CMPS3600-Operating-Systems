/*
 *  echo.c
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
 *    $ gcc -o echo echo.c -lpthread   # link in POSIX pthread library  
 *    $ ./echo 
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

#define DEBUG 0 

/* thread function prototypes */
void consumer(void *dummy); 

/* A note on the differences between forks and threads. Variables local to 
 * main that exist before the fork are inherited by the child but not shared.
 * Threads, since they are functions, can only see globals. These globals are
 * not only visible but shared by all threads since threads share user space.
 */

int retval;
const int LIMIT = 55;  /* for testing read 55 chars from the file */
char buf[1];           /* 1 char buffer */
int fib(int);
FILE * inf;

int main(void)
{
	pthread_t cthr;  /* cthr will point to the consumer thread */
	int dummy;

	/* we are using formatted fopen(2) to make life easier - normally
	 * in systems coding you would use open(2)
	 */
	inf = fopen("poem", "r");
	if (!inf) {
		fprintf(stderr, "error opening input file.\n");
		exit(1);
	}

	/* create consumer thread */
	if (pthread_create(&cthr, NULL,  (void *)consumer, (void *)&dummy) != 0)
		fprintf(stderr,"Error creating thread\n");

	buf[0] = ' ';
	pid_t tid = syscall(SYS_gettid);
	if (DEBUG)
		printf("main thread pid: %d tid: %d \n",getpid(), tid);

	/* PRODUCER 
	 * reads 1 char from poem file and writes that char to buffer */
	int count = 0;
	while (1) {
		if (count == LIMIT)
			break;
		/* critical code */
		buf[0] = fgetc(inf);  
		if (DEBUG)
			putc(buf[0],stdout);  
		fib(16);    /* make the producer slightly slower than consumer */
		count++;
	}

	/* the parent thread always joins with its spawned threads */
	if ((pthread_join(cthr, (void*)&retval)) < 0) 
		perror("pthread_join");
	else  
		if (DEBUG) printf("joined consumer thread w exit code: %d\n",retval);

	/* parent closes input file */
	fclose(inf); 
	exit(0);
}

/* CONSUMER thread function 
 * reads 1 char from shared buffer and writes char to screen 
 */
void consumer(void *dummy)
{
	int count = 0;
	pid_t tid = syscall(SYS_gettid);      
	fprintf(stdout,"consumer thread pid: %d tid: %d \n",getpid(), tid);
	while (1) {
		if (count == LIMIT) 
			break;
		fputc(buf[0],stdout);
		fib(15);   /* make consumer slightly faster than producer */
		count++; 
	}
	fputc('\n',stdout);
	pthread_exit(0);
}

int fib(int n)
{
	if (n == 1)
		return 1;
	if (n == 2)
		return 1;
	return fib(n-1) + fib(n-2);
}
