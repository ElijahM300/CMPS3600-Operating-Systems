/*
 * sample3.c
 *
 * + demonstrate use of pthread_mutex to synchronize thread behavior 
 * + put a mutex on the screen to ensure I/O statements are not interrupted 
 * + use structure to keep exit code for each thread 
 * + there is a race condition as to who grabs mutex next but ok for now 
 * 
 * $ gcc -o sample3 sample3.c -lpthread
 * $ ./sample3
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_THREADS 5 

/* this will hold the thread ids */
pthread_t tids[NUM_THREADS];

typedef struct {
	int thread_num;
	int thread_exit_code;
} thread_data;

/* call to do some busy work for sanity check on synchronization */
int fib(int n)
{
	if (n == 1 || n == 2)
		return 1;
	return fib(n-1) + fib(n-2);
}

/* create a structure to hold thread info */
thread_data my_data[NUM_THREADS];

/* mutex identifier needs to be global so threads can see it */
pthread_mutex_t my_mutex;

/*-----------------*/
/* THREAD FUNCTION */
/*-----------------*/
void printIt(void *thread_num) {
	/* pointers are longs */
	long tnum = (long)thread_num;  

	/* determine who you are by thread id */
	pthread_t self = pthread_self();

	/* --- CRITICAL CODE SECTION ----*/
	pthread_mutex_lock(&my_mutex);
	if (pthread_equal(self,tids[0])) {
		printf("Goodbye ");
	} else {
		printf("Hello ");
	}

	/* if tnum is even then do some busy work to test mutex lock */
	if ((tnum % 2) == 0) {
		fib(37);
	}
	printf("from thread #%ld!\n",tnum);
	my_data[tnum].thread_exit_code = ((tnum*23)+1)%7;
	pthread_mutex_unlock(&my_mutex);
	/* ----  END CRITICAL CODE  ----- */

	pthread_exit(&my_data[tnum].thread_exit_code);
} 


int main(int argc, char *argv[])
{
	int rc;
	long t;

	/* create a mutex with default attributes */
	rc = pthread_mutex_init(&my_mutex, NULL);
	if (rc) { 
		printf("ERROR; return code from pthread_mutex_init() is %d\n",rc);
		exit(-1);
	}

	for (t=0; t<NUM_THREADS; t++) {
		rc = pthread_create( &tids[t], NULL, (void *)printIt, (void *)t);
		if (rc) { 
			printf("ERROR; return code from pthread_create() is %d\n",rc);
			exit(-1);
		}
		pthread_mutex_lock(&my_mutex);
		printf("just created thread #%ld\n",t);
		pthread_mutex_unlock(&my_mutex);
	}

	/* join with all threads - actual thread exit order doesn't matter
	 * but parent will block until thread 1 exits then thread 2 and so on...
	 */
	int *ptr = &my_data[0].thread_exit_code;
	for (t=0; t<NUM_THREADS; t++) {
		rc = pthread_join(tids[t], (void **)&ptr);
		if (rc) {
			printf("ERROR; return code from pthread_join() is %d\n",rc);
			exit(-1);
		}
		pthread_mutex_lock(&my_mutex);
		printf("just joined with thread #%ld: thread's exit code: %d\n",t,*ptr);
		pthread_mutex_unlock(&my_mutex);
	}

	/* now parent can exit its own thread */
	pthread_mutex_destroy(&my_mutex);
	pthread_exit(NULL);
}
