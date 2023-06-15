/*
 * sample2.c
 *
 * demonstrate use of pthread_join to synchronize thread behavior
 *
 * the potential exists for one thread to clobber the exit code of another
 * thread - we will fix that later
 * 
 * $ gcc sample2.c -osample2 -lpthread
 * $ ./sample2
 *
 *   A double pointer is a pointer variable that holds the address
 *   of another pointer. It usually means there is an array of pointers.
 *
 *   The array will have its own address, and will also contain addresses.
 *
 *   This code uses double pointers.
 *   To review, study and modify the
 *
 *       test_double_pointers()
 *
 *   function just below.    
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_THREADS 5 

void test_double_pointers(void)
{
	int code;    /* an integer                            */
	int *ptr;    /* a pointer to hold the address of code */
	int *arr[1]; /* an array of pointers to hold ptr      */

	printf("test_double_pointers()...\n");
	printf("all 5's should be displayed.\n");
	code = 5;
	ptr = &code;
	arr[0] = ptr;
	printf("code: %i\n", code);
	printf("code: %i\n", *ptr);
	printf("code: %i\n", *(arr[0]));
	printf("code: %i\n", *(*arr));
	printf("test complete.\n\n");
}


/* use fib to make threads do some busy work
   and do a sanity check on join */
int fib(int n)
{
	if (n == 1)
		return 1;
	if (n == 2)
		return 1;
	return fib(n-1) + fib(n-2);
}

int thread_exit_code = 0;
pthread_t tids[NUM_THREADS];

void printIt(void *thread_num)
{
	/* ------- THREAD_FUNCTION -------- */

	/* pointers are longs so coerce to long */
	long tnum = (long)thread_num; 

	/* do some busy work */ 
	fib(20 + ((tnum*10)%7));

	printf("hello there from thread #%ld!\n", tnum);

	/* demonstrate that thread IDs are reused */
	pthread_t self = pthread_self();

	/* this statement is always true */
	if (pthread_equal(self, tids[0])) {
		thread_exit_code = tnum+99;
	} else {
		printf("---> not equal\n");
		thread_exit_code = tnum+10;
	}
	pthread_exit(&thread_exit_code);
} 


int main(int argc, char *argv[])
{
	int rc;
	long t;
	test_double_pointers();
	for (t=0; t<NUM_THREADS; t++) {
		rc = pthread_create(&tids[t], NULL, (void *)printIt, (void *)t);
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n",rc);
			exit(-1);
		}
		printf("just created thread #%ld\n",t);
		fflush(stdout);

		/* wait for thread t to terminate before creating the next one 
		 * when you do this the thread ID is re-used! */
		int ret_code;
		int *ptr = &ret_code;
		rc = pthread_join(tids[t], (void **)&ptr);
		if (rc) {
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
		printf("just joined with thread #%ld ", t);
		printf(" : thread's exit code: %d\n", *ptr);
	}

	/* now parent can exit its own thread */
	pthread_exit(NULL);
}

