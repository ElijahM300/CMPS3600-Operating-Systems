/* handler_masks.c

   Demonstrate use of signal masks to block signals while in a signal handler.

   Uses signal handling to trap CTRL-C.

   Compile and execute. Hit CTRL-C. Note that with the signal handler, CTRL-C 
   breaks you out of the loop but allows you to continue executing. Hit Ctrl-C
   while in the handler. The handler blocks it. As soon as you exit the
   handler the zero mask is restored, the Ctrl-C is resent from the queue
   and is trapped the 2nd time. 

   Some notes: the use of keyword 'volatile' tells the compiler that the value
   of the variable must be grabbed from its memory address and not from cache. 
   This protects the value from strange things happening during a signal 
   handler. The 'sig_stomic_t' type is an integer type that is accessed to 
   read or write in a single atomic action (hence cannot be interrupted) 

   The include file errno.h lets you call systems level error handling routines.
   In particular, perror(), which accesses the global variable errno that holds
   the error code of the last failed system call. See /usr/include/sys/errno.h
   for predefined error codes. Here are a few:

#define EPERM  1  // not owner 
#define ENOENT 2  // no such file or directory
#define ESRCH  3  // no such process
#define EINTR  4  // interrupted system call 
#define EIO    5  // I/O error 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/file.h>

/* use 'volatile sig_atomic_t' type for data you want to access in handlers */
volatile sig_atomic_t got_ctrlC = 0; 

/* the signal handler function prototype is: void my_handler(int) */
void my_handler(int sig)
{
	got_ctrlC++;
	char istr[3];
	/* sprintf can convert int to string */ 
	sprintf(istr,"%d", got_ctrlC);  /* reads from integer to a string */
	if (got_ctrlC == 1) {
		write(1, "\nCTRL-C grabbed from main: ", 27); 
		strcat(istr,"\n");
		write(1, istr, 3); 
		write(1, "I'm blocking CTRL-C for 5 secs. Hit CTRL-C to show it.\n",57); 
		sleep(5);   /* show that CTRL-C is blocked while in handler */
	} 
	/* this signal trapped from the signal queue after leaving the handler */  
	if (got_ctrlC == 2) {  
		write(1, "\nCTRL-C grabbed from queue: ", 28); 
		strcat(istr, "\n");
		write(1, istr, 3); 
		return; 
	}
}

int main(void)
{
	struct sigaction sa;
	sa.sa_handler = my_handler;
	/* set to SA_RESTART if you want to restart a system call in handler */
	sa.sa_flags = 0;
	/* sa.sa_mask is the mask used in handler -
	 * mask returned to original value upon exit */
	sigemptyset(&sa.sa_mask);
	/* block CTRL-C while in handler */
	sigaddset(&sa.sa_mask, SIGINT);
	/* register the signal handler for the signal */ 
	if (sigaction(SIGINT, &sa, NULL) == -1) {  /* failed calls return -1 */ 
		perror("sigaction");   /* perror grabs the error and displays it */ 
		exit(1);
	}
	while (1) {
		printf("HELP! I'm stuck in an infinite loop. Hit Ctrl-C!\n");
		sleep(1);
		if (got_ctrlC)
			break;
	}
	printf("\nThanks. I needed that.\n");
	return 0;
}
