/*
 *  sighand1.c
 * 
 *  A gentle introduction to the use of signal handling.
 *  Uses signal handling to trap Ctrl-C (SIGINT) and Ctrl-Z (SIGTSTP).
 * 
 *  Compile and execute. Hit Ctrl-Z followed by Ctrl-C. Note that with this
 *  signal handler, only CTRL-C breaks you out of the loop. 
 *
 *  Some notes: the use of keyword 'volatile' tells the compiler that the value
 *  of the variable must be grabbed from its memory address and not from cache. 
 *  This protects the value from strange things happening during a signal 
 *  handler. The 'sig_atomic_t' type is an integer type that is accessed to 
 *  read or write in a single atomic action (hence cannot be interrupted) 
 *
 *  The 'sa_mask' in the sigaction struct (shown below) is the mask of signals
 *  that should be blocked during execution of the signal handler.
 *
 *          struct sigaction {
 *               void     (*sa_handler)(int);
 *               void     (*sa_sigaction)(int, siginfo_t *, void *);
 *               sigset_t   sa_mask;
 *               int        sa_flags;
 *               void     (*sa_restorer)(void);
 *         };
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/file.h>


/* note: to be really safe variables set by handler should be:
 * volatile sig_atomic_t flag = 0; 
 */

/* make this global so the handler can set it */
int my_flag = 0; 

void myhandler(int sig)
{
	/* signal handler */
	/* Ctrl-Z */
	if (sig == SIGTSTP)
		write(2, "just got Ctrl-Z\n", 16);
	/* Ctrl-C */
	if (sig == SIGINT) {
		write(2, "just got Ctrl-C\n", 16);
		my_flag++;
	}
}

int main(void)
{
	char buf[50];
	int ret;
	sigset_t mask;  
	sigfillset(&mask);  /* create a full mask */
   	/* block all signals until handler is setup */
	sigprocmask(SIG_BLOCK, &mask, NULL);
	/* set stdout to not buffer */
	setvbuf(stderr, NULL, _IONBF, 0);

	struct sigaction sa;  /* sa is a variable name for a sigaction structure */

	sa.sa_handler = myhandler; /* point it at your handler function */

   	/* restart interrupted system calls - set to 0
	 * if you do not want to restart */
	sa.sa_flags = SA_RESTART;

	/* sa.sa_mask is mask used in handler; previous process mask is restored
	 * upon handler exit; a full set means all signals are blocked - empty
	 * set means no signals are blocked; by default the handler signal(s) are
	 * blocked */
	sigfillset(&sa.sa_mask);

	/* link the signal handler to the SIGTSTP and SIGINT signal */ 
	ret = sigaction(SIGTSTP, &sa, NULL);
	ret = sigaction(SIGINT, &sa, NULL);
	if (ret == -1) {
		perror("sigaction");
		exit(1); 
	}   
	sigemptyset(&mask);   /* empty mask - do not block any signals */
	strcpy(buf, "hit Ctrl-Z followed by Ctrl-C\n");
	write(1, buf, strlen(buf));

	/* put yourself into sigsuspend(2) waiting for Ctrl-C */
	while (1) {
		sigsuspend(&mask);  /* process mask changed to mask so any signal will 
		* break you out of sigsuspend but only sigint will 
		* break you out of the loop */
		if (my_flag)
			break;
	}
	strcpy(buf, "Just made it out of the loop!\n");
	write(1, buf, strlen(buf));
	return 0;
}

