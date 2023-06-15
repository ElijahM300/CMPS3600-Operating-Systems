/* 
 * threads1.c
 *
 *  Demonstrate passing a structure to thread, grabbing thread ID and
 *  grabbing thread return code
 *
 *  Uses these calls:
 *  int pthread_create(pthread_t *restrict thread, 
 *           *const pthread_attr_t *restrict attr, 
 *           void *(*start_routine)(void*), void *restrict arg);
 *
 *  int pthread_join(pthread_t thread, void **retval);
 *    pthread_t pthread_self(); 
 *
 *  void pthread_exit(void *retval);
 *
 *    $ gcc -o threads1 threads1.c -lpthread   
 *    $ ./threads1
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>  /* you need this for POSIX threads */
#include <unistd.h>

typedef struct {
	int i;
	double d;
} a_struct ;

int retval; /* default location for return value of a thread */ 

void thread_func(void *);

int main(void)
{
	pthread_t thread1;

	a_struct mystuff;
	mystuff.i = 10;
	mystuff.d = 10.5;

	/* create thread1, use default attributes, pass a structure to the func*/
	if (pthread_create(&thread1, NULL, (void *)thread_func,
			(void *) &mystuff) < 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	/* when you join the thread grab the return value */
	if ((pthread_join(thread1, (void*)&retval)) < 0)
		perror("pthread_join");
	else
		printf("joined thread1 exit code: %d\n",retval);
	exit(EXIT_SUCCESS);
}


void thread_func(void *ptr)
{
	pthread_t me = pthread_self(); /* grab your thread id - it is an opaque
									  type but read pthreads.h - type unsigned
									  long int under linux */

	a_struct *tmp = ptr; /* do this since a void ptr cannot be dereferenced */

	printf("(%li) int: %d  double: %f \n", me, (*tmp).i, (*tmp).d);

	/* terminate this thread and stuff exit code into retval */
	pthread_exit(EXIT_SUCCESS);
}

