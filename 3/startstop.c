/*  startstop.c 
 *  Goal: understand the use of signal masks and handlers
 *
 *  This source has extensive comments - you will see fewer and fewer as
 *  the quarter progresses. Read and understand everything now. 
 *  
 *  This program demonstrates POSIX signal handling, race conditions and 
 *  the sigsuspend(2) call:
 *
 *        int sigsuspend(const sigset_t *mask);
 *
 *  sigsuspend(&mask) temporarily replaces the signal mask of the calling 
 *  process with mask and then suspends the process until delivery of an
 *  unblocked signal whose action is to invoke a signal handler if there is
 *  one or to perform the default behavior of the signal (which is generally 
 *  to terminate the process).  
 *
 *  The process periodically displays a dot if DOTS flag is set. The process is 
 *
 *  1. effectively PAUSED (suspended) with SIGUSR1 (kill -USR1 [pid]) ; the
 *     process waits at sigsuspend() in usr1_handler() for a signal.
 *
 *  2. restarted UNPAUSED with SIGUSR2 (kill -USR2 [pid])
 *
 *  3. terminated with SIGINT (CTRL-C from keyboard)
 *
 *  4. impacted by a race condition; if you give two signals on the same line: 
 *
 *            $ kill -USR1 [pid] ; kill -USR2 [pid]
 *
 *    you cannot guarantee which signal is delivered first. Two closely spaced 
 *    signals are NOT guaranteed to be delivered in order by the kernel (a
 *    real-time kernel will guarantee this). 
 *
 *    Of the 32 available signals, the Unix kernel reserves all but SIGUSR1 and 
 *    SIGUSR2. These two signals can be used to do anything. 
 *
 *    There is no fork so there is only ONE process signal mask in effect at
 *    any given time. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <math.h>
#include <nl_types.h>
#include <string.h>
#include <errno.h>

/* The define preprocessor directive is a macro for readability - ADD_NICE 
 * references will be replaced by 19 by pre-processor before compilation 
 */

#define DOTS 0        /* DOTS=1 to display dots; DOTS=0 to not display dots */

#define ADD_NICE 19   /* reduce priority by 19; large numbers have lower 
						 priority than smaller ones */

pid_t  my_pid;       /* use this to identify yourself */


/* sig_atomic_t is implemented as some integral type (int,long,short); use
 * sig_atomic_t type (rather than unsigned int for example) for portability
 * 
 * the volatile keyword tells compiler to translate assignments into
 * instructions that are completed atomically; i.e., as if it were a 
 * single instruction. 
 */

volatile sig_atomic_t paused = 0;  /* 0 means process is NOT paused */

/*  The next three variables will hold signal masks; a mask defines the
 *  the signals that you want to do something with, typically which signals
 *  to block (ignore) or which signals to accept; the sigset_t structure 
 *  looks like this:                      see /usr/include/bits/sigset_t.h
 *          typedef struct
 *          {
 *              unsigned long int __val[_SIGSET_NWORDS];
 *          } __sigset_t; 
 *
 *  Each process has a signal mask. By default (i.e., before you change it)
 *  the signal mask of a process is the zero mask (i.e., no signals blocked). 
 */

sigset_t zeromask;       /* holds a zero mask that includes NO signals */
sigset_t handlermask;    /* block these signals while in interrupt handler */
sigset_t savedmask;      /* original mask */

/* The next 3 prototypes define interrupt handlers; an interrupt handler 
 * works like this: if you get an unblocked signal that has a handler you do 
 * an unconditional jump to the handler and execute the code in the handler 
 * function; depending on how you code the handler you can either exit() the 
 * program from the handler or return back to the calling routine on the stack
 */

void usr1_handler(int);     /* handler for USR1 signal - pauses process */ 
void usr2_handler(int);     /* handler for USR2 signal - restarts process */
void ctlc_handler(int);     /* handler for CTRL-C generated at keyboard */

int main(int argc, char *argv[], char *envp[])
{
	int count = 1;

	if (nice(ADD_NICE) < 0)  /* reduce priority; see PRI in ps -lyu {user} */
	{  perror("nice: ");     /* perror displays the current error code to stderr*/
		exit(-1);
	}

	my_pid = getpid();      /* grab your process id so you can display it */

	/*  Setup your signal masks.
	 *  A signal mask is a set (a bit string of length 32 where 1 in position
	 *  7 (for example) means signal 7 is in the set and 0 means signal 7 is
	 *  not; you start with an empty set and add signals to it; you are not yet
	 *  assigning the mask to the process - you do that later by sigprocmask()
	 */ 
	sigemptyset(&zeromask);  /* zeromask is the empty set - no signals in it */
	sigemptyset(&savedmask);   /* use this to hold original mask */
	sigemptyset(&handlermask);  /* set this mask insdie the interrupt handlers */

	/* add three signals to the handlermask */
	sigaddset(&handlermask, SIGUSR1); /* SIGUSR1 - 1 of 2 user defined signals */
	sigaddset(&handlermask, SIGUSR2); /* SIGUSR2 - 2 of 2 user defined signals */
	sigaddset(&handlermask, SIGINT);  /* SIGINT ; CTRL-C from the keyboard */

	/* define structures for the signals you want to handle */
	struct sigaction usr1;  
	struct sigaction usr2;  
	struct sigaction ctlc;  

	/* assign the handler routines to the appropriate signal */
	usr1.sa_handler = usr1_handler; 
	usr2.sa_handler = usr2_handler; 
	ctlc.sa_handler = ctlc_handler; 

	/* restart interrupted system calls in usr1 and usr2 handlers */
	usr1.sa_flags = SA_RESTART; 
	usr2.sa_flags = SA_RESTART; 

	/* register each handler */ 
	sigaction(SIGUSR1, &usr1, NULL);  
	sigaction(SIGUSR2, &usr2, NULL);  
	sigaction(SIGINT,  &ctlc, NULL);  

	/* put the process in an infinite loop; hit ctrl-c to exit */

	fprintf(stdout, "\nMy PID is: %d\n", my_pid);
	fprintf(stdout,
				"Send SIGUSR1 to pause, SIGUSR2 to cont or Ctrl-C to stop\n");
	while (1) {
		if (DOTS) {
			usleep(1200000);  /*usleep is microseconds - this is 1.2 seconds ;
								 use usleep or sleep to slow display down */
			fprintf(stdout, "."); 
			if (count % 60 == 0) fprintf(stdout, "\n");
			fflush(stdout); 
			count++;
		}
	}
	return(0);
}  
/* END MAIN */


/***********************
 *  SIGUSR1 HANDLER    *
 ***********************/

/* the SIGUSR1 signal handler sets paused flag to 1, enters a suspended state 
 * until some other signal (e.g. SIGUSR2) breaks sigusr1 out of sigsuspend 
 * AND if the handler for that signal set paused flag to 0 
 */

void usr1_handler(int signo) 
{
	if (paused == 1) {  /* a USR1 signal has already been delivered */
		write(1, "\n(usr1handler) oops! process already paused.\n",45);
		return;
	} else  {

		/* Temporarily block incoming signals; by default the signal that 
		 * triggered the handler is blocked before the handler is executed 
		 * but it doesn't hurt to include it in the set. The call is:
		 *
		 * sigprocmask(int options, *set, *oldset);  
		 *  options:
		 *  SIG_BLOCK    - you can block everything except SIGKILL and SIGSTOP
		 *                 (these signals in the mask are ignored)
		 *  SIG_UNBLOCK  - take the signals in the set out of the process mask
		 *  SIG_SETMASK  - the set of currently blocked signals is added to set
		 *
		 * sigprocmask copies current mask to savedmask and sets process mask 
		 * to handlermask, effectively blocking signals in the handler before 
		 * executing critical code 
		 */

		sigprocmask(SIG_BLOCK, &handlermask, &savedmask);   

		/* CRITICAL CODE SECTION
		 * you can now safely execute critical code since 
		 * signals are blocked and thus you will not be interrupted 
		 */

		paused = 1;  /* set flag so usr2 handler knows usr1 handler has 
					  * suspended things */

		/* note: printf is unsafe for signal handlers; see signal(7) for a list
		 * of async-safe functions; write is async-safe so use it
		 */

		write(1,"(usr1handler) pausing\n",22);

		while (paused == 1) {  
			sigsuspend(&zeromask);   /* a busy wait: wait for any signal to 
									  * come in but only USR2 can break you 
									  * out of the loop since only USR2 can
									  * set flag 
									  */
		}
		/* if we made it here we know the process got a USR2 signal and the
		 * the usr2_handler set paused to 0. we also know that sigsuspend()
		 * before returning restored the process signal mask to the state
		 * before sigsuspend; i.e. the same as doing this: 
		 *             sigprocmask(SIG_SETMASK, &handlermask, NULL); 
		 */

		if (write(1, "(usr1handler) exiting\n",22) < 0) perror("write: ");

		/* Before leaving restore the signal mask to what it was before the
		 * usr1_handler was executed; i.e., unblock all signals except sigusr1
		 */ 
		sigprocmask(SIG_SETMASK, &savedmask, NULL); 

	} /* END ELSE */
	return;
}  


/***********************
 *  SIGUSR2 HANDLER    *
 ***********************/

/* the purpose of this handler is to set paused=0 so usr1_handler can 
 * break out of its while(paused) loop 
 */

void usr2_handler(int signo) 
{
	if (paused == 0) 
	{
		if (write(1, "(usr2handler) oops! process already unpaused.\n",46) < 0)
			perror("write: ");
		return;
	}
	else {
	   	/* block usr1,usr2, int signals */
		sigprocmask(SIG_BLOCK, &handlermask, &savedmask);

		/* CRITICAL CODE SECTION */
		if (write(1,"(usr2handler) unpausing\n",24)< 0) perror("write: ");

		paused = 0;  /* flip flag so usr1_handler can break out of sigsuspend */

		if (write(1, "(usr2handler) exiting\n",22) < 0) perror("write: ");

		sigprocmask(SIG_SETMASK, &savedmask, NULL); /* restore original mask */

		return;
	}
}  


/**********************
 *  SIGINT HANDLER    *
 **********************/

/* the SIGINT handler takes care of  CTRL-C generated at the keyboard */

void ctlc_handler(int signo)
{
	write(1, "\nSIGINT received. You killed me!\n",36);
	exit(0);    /* stop program */
}

