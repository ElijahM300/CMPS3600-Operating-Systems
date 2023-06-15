/* filename: sigset.c 
 * 
 *  Demonstrate POSIX signal sets and signal handling routines.
 *  Demonstrate how to handle a signal by grabbing it from the pending
 *  signal set and ignoring it.
 *
 *  POSIX is a standard interface across all flavors of UNIX that allows C 
 *  system code to be portable and readable. 
 *
 * A signal set lets you more easily implement signal handling by treating 
 * multiple signals (a signal set) in the same way. Start by reading the 
 * manpages for POSIX signal set manipulation:
 *  
 *    $ man 3 sigsetops    // will show all signal operations 
 *    $ man sigprocmask    // examine and change process mask 
 *
 * The POSIX signal API includes the following routines for signal sets. Like
 * most Unix routines (unless stated otherwise) these will return 0 on success 
 * and -1 on error. The type 'sigset_t' is a signal set type. It is a 32-bit 
 * integer manipulated at the bit level; i.e., a string of 32 flag bits where
 * 1 means the signal is in the set and 0 means the signal is not. Integers that
 * are treated as such are called masks. See signal.h and /bits/signum.h
 *
 * int sigemptyset(sigset_t *set); # initialize set to include no signals 
 * int sigfillset(sigset_t *set  ); # initialize set to include all signals
 * int sigaddset(sigset_t *set,int signum); # add signal signum to the set 
 * int sigdelset(sigset_t *set,int signum); # delete signal signum from set 
 * int sigismember(sigset_t *set, int signum); # return 1 if signum is in set 
 * int sigpending( sigset_t *set); # check if any signals is sigset are pending 
 *
 * After you create the signal set, you need to process the mask, where 'how' 
 * can be one of SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK.
 *
 * int sigprocmask(int how, const sigset_t *set, sigset_t *oldset); 
 *
 * Next register the handler. See week02 for a review of using this call:
 *
 * int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
 *
 * The 'sa_mask' in the sigaction struct (shown below) gives  a mask of signals 
 * which should be blocked during execution of the signal handler.   
 *
 *          struct sigaction {
 *               void     (*sa_handler)(int);
 *               void     (*sa_sigaction)(int, siginfo_t *, void *);
 *               sigset_t sa_mask;
 *               int      sa_flags;
 *               void     (*sa_restorer)(void);
 *           };
 */

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

void my_handler(int sig);
void err_quit(char *);

int main(void) {

	sigset_t set, pending;

	/* start with an empty signal set */
	if (( sigemptyset(&set)) < 0 )
		err_quit("sigemptyset");

	/* add SIGTERM to the empty signal set */
	if (( sigaddset(&set,SIGTERM)) < 0 )
		err_quit("sigaddset");

	/* just for kicks check if SIGTERM is there */
	if ( sigismember(&set,SIGTERM) )
		puts("SIGTERM is in signal mask");
	else
		puts("SIGTERM is not in signal mask");

	/********************************************************************* 
	 * Error handling omitted after here for clarity. You should include it!
	 ***********************************************************************/

	sigaddset(&set,SIGALRM); /* add SIGALRM */
	if (sigismember(&set,SIGALRM)) 
		puts("SIGALRM is in signal mask");
	sigdelset(&set,SIGALRM); /* delete SIGALRM */
	if (!sigismember(&set,SIGALRM)) 
		puts("SIGALRM is not in signal mask");

	sigprocmask(SIG_BLOCK, &set, NULL);  /*add signals in set to the current
											set of blocked signals; 
											a blocked signal is delivered to
										   	the	pending set -
										   	it is not discarded */

	kill(getpid(), SIGTERM); /* send the blocked signal to yourself */
	sigpending(&pending);  /* get pending signals */

	/* if SIGTERM pending, make a handler */
	struct sigaction action;

	if (sigismember(&pending,SIGTERM)) 
	{
		puts("SIGTERM is pending..\n");
		action.sa_handler = SIG_IGN; /* the handler is 'ignore the signal'  */
		sigaction(SIGTERM, &action, NULL);  /* register the handler */
	}

	/* unblock SIGTERM - the pending signal is handled by ignoring it ; after
	 * ignoring it, the signal is discarded -  
	 */
	sigprocmask(SIG_UNBLOCK, &set, NULL);

	/* now setup your own handler */
	action.sa_handler = my_handler;
	sigfillset(&action.sa_mask);   /* block all signals while in handler */
	sigaction(SIGTERM, &action, NULL);  /* re-register the handler */
	kill(getpid(), SIGTERM);            /* send the signal to yourself again */

	exit(EXIT_SUCCESS);
}

void err_quit( char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

/* we are only only handling one signal - SIGTERM - so 'sig' is not used -
 * you can use the same routine to handle multiple signals - sig holds the
 * signal number of the signal that triggered the handler 
 */

void my_handler(int sig)
{
	if ( sig == SIGTERM )
		write(1, "finally got SIGTERM \n", 22); //22 is how many chars to print 
	else 
		write(1, "this should never execute\nn", 30);   
}

