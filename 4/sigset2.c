/* sigset2.c   - WEEK 4 -    
 *   
 * Use POSIX signal sets to facilitate signals between parent and child 
 *
 * In this program, the child sends a SIGCHLD signal to the parent , the parent
 * kills the child, which the child handles properly .
 *
 * note: no error handling is done in the signal handling code for brevity
 *              DO NOT OMIT ERROR HANDLING IN YOUR CODE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

void handler(int sig);  /* use the same handler for both parent and child */

int status; 

int main()
{
	pid_t parent = getpid(); 

	/* spawn a child */
	int child = fork();

	/* PARENT CODE */
	if ( getpid() == parent ) {
		/* this parent has its own sigaction structure */
		sigset_t set, pending;
		struct sigaction action;     

		sigemptyset(&set);
		sigaddset(&set,SIGCHLD); 
		sigprocmask(SIG_BLOCK, &set, NULL); 

		sigemptyset(&action.sa_mask); 
		action.sa_handler = handler;    
		sigaction(SIGCHLD, &action, NULL); 

		sleep(4);  /* sleep long enough to let the child send a signal */
		kill(child,SIGHUP);   /* kill the child before handling the signal */
		sleep(2);
		sigpending(&pending);  /* get pending signals */
		if (sigismember(&pending,SIGCHLD)) { 
			sigprocmask(SIG_UNBLOCK, &set, NULL);
		}
		exit (EXIT_SUCCESS);
	}

	/* CHILD CODE */
	if ( child == 0 ) {
		int parent = getppid();   /* grab the parent's pid */

		sigset_t set;
		(void)set;
		struct sigaction action;     
		sigemptyset(&action.sa_mask); 
		action.sa_handler = handler;    
		sigaction(SIGHUP, &action, NULL); 

		kill(parent,SIGCHLD);  /* send SIGCHLD to the parent */
		while (1) {
			/* loop until the parent kills you */
		} 
	}
	return 0;
}

void handler(int sig)
{
	if (sig == SIGHUP)
		write(1, "OK What is it?\n", 15);  /* write is async safe, 15 is the 
											  number of characters to write */ 
	else if (sig == SIGCHLD) 
		write(1, "I just died.\n", 15);  
}
