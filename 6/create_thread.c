/* create_thread.c
 *
 * Demonstrate use of pthread_create() function.
 *
 *  Compile and link:
 *    $ gcc -o create_thread create_thread.c -lpthread  #link in pthread library
 *    $ ./create_thead
 *
 *  Read pthread_create() manpage for more detailed information.
 *
 *  The function prototype from /usr/include/pthread.h is:
 *
 *  int pthread_create(pthread_t *restrict thread,
 *              const pthread_attr_t *restrict attr, 
 *              void *(*start_routine)(void*), void *restrict arg);
 *
 *  o 'thread' references the ID of created thread - 'restrict' type qualifier
 *    prevents the refererent from being aliased by another pointer
 *  o use NULL for 'attr' for default attributes 
 *  o upon creation, executes start_routine - function parameters for this
 *    routine must be void * return and void * parameter; 
 *  o arg is the sole argument to the start routine 
 *  o upon return from start_routine, an implicit call to pthread_exit() 
 *    removes thread 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void * print_msg(void *ptr);

int main(void)
{
	pthread_t thread1, thread2;

	char *msg1 = "Hello     ";
	char *msg2 = "World   \n";

	/* create thread1, use default attributes, pass print_msg function */
	if (pthread_create(&thread1, NULL, print_msg, (void *)msg1) < 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	} 

	/* At this point there are two threads of execution within one process.
	 * While both threads are equal, we refer to the original thread (created 
	 * when you load the program into memory) as the parent thread. All threads
	 * spawned from the orignal thread are offspring of that thread. In this
	 * simple example each offspring is executing the same code (the function
	 * print_msg) but with different command-line arguments. 
	 */

	/* spawn a third thread of execution: */
	pthread_create(&thread2, NULL, print_msg, (void *) msg2);

	/* pthread_join blocks parent until the threads finish */
	if ((pthread_join(thread1, NULL)) < 0) {
		perror("pthread_join");
		exit(EXIT_FAILURE);
	}
	if ((pthread_join(thread2, NULL)) < 0) {
		perror("pthread_join");
		exit(EXIT_FAILURE);
	}

	/* calling exit() by any thread will terminate all threads since exit(2)
	 * is a process-wide call 
	 */
	exit(0);
}


/* you must pass void * to the thread function but you can cast any pointer to
 * void * so effectively you can pass anything  - it is up to the programmer
 * to cast the argument back to its original type */ 
void * print_msg(void *ptr)
{
	char *message;
	message = (char *) ptr;  /* cast the argument back to its real type */
	write(1,message,10);

	/* call pthread_exit() to terminate this thread only */
	pthread_exit(0);
}

