/*
 * sample1.c
 *  
 * demonstrates how to create threads 
 * 
 * $ gcc sample1.c -lpthread
 * $ ./a.out
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>  
#include <stdlib.h>

#define NUM_THREADS 5

void printIt(void *thread_num);

int main( int argc, char *argv[])
{
	int rc;
	long t;
	pthread_t threads[NUM_THREADS];

	for (t=0; t<NUM_THREADS; t++) {
		printf("creating thread %ld\n", t);
		/* pthread_create stuffs thread id into threads array */
		rc = pthread_create(&threads[t], NULL, (void *)printIt, (void *)t);
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	for (t=0; t<NUM_THREADS; t++) {
		if (pthread_join(threads[t], NULL) < 0) {
			perror("pthread_join: ");
		}
		printf("joined thread %ld\n", t);
	} 
	/* parent can now exit its own thread */
	pthread_exit(NULL);
}

void printIt(void *thread_num)
{
	/* type cast the assigment into a long */
	/* because the value is stored in a pointer */
	long tnum = (long)thread_num;

	/* demonstrate process-wide syscall.
	   it prevents all threads from joining
	   because this thread is still running */
	if (tnum == 0) {
		printf("thread %ld is sleeping...\n", tnum);
		sleep(6);
	}

	printf("hello there from thread #%ld!\n", tnum);
	pthread_exit(NULL);
}

