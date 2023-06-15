/*
 *  sigmask.c
 *
 *  Demonstrate sleep(3) and the use of SIGINT, SIGSTOP, SIGCONT and SIGTERM 
 *  and setting a signal mask to block all signals.
 *
 *  Compile
 *
 *      $ gcc -o sigmask sigmask.c
 *      $ ./sigmask   # hit CTRL-C
 *  
 *  Open another terminal window. Run top in that window:
 *
 *       $ top -u [username] -d .5    # -d specifies .5 second delay 
 *
 *  We want to view all CPUs. Hit 'f'. Arrow down and over to the P (Last Used 
 *  CPU) field. Hit space to toggle P on. Hit 'q'. Now view the process state 
 *  column (S). The ./sig1 and top processes move from R=running to S=sleeping. 
 *  The S is a little misleading. Your other processes are not sleeping at all
 *  times. Top is taking a sampling of CPU cycles. It looks like top or ./sig1
 *  are running continuously. Other processes on the shared CPUs are in fact in
 *  state R but the time slice is so quick that you do not see it.  
 *
 *  Note that Ctrl-C generates a SIGINT signal to the process linked to the 
 *  tty/keyboard that you hit Ctrl-C on. In top the process silently dies. 
 *  You could also hit 'k' from top and specify to PID. Hit 'q' to exit top.
 *   
 */

#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   /* this header file is for the POSIX interface */
#include <string.h>

int main(int argc, char*argv[])
{
	sigset_t mask;
	sigemptyset(&mask);  
	/* add SIGINT to mask */
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	write(1, "Hit Ctrl-C! (within 5-seconds please)\n", 38);
	/* SIGINT is blocked so nothing happens for 10 seconds */
	sleep(5);
	/* now Ctrl-C is unblocked */
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	printf("This code is never executed if Ctrl-C is hit.\n");
	return 0;
}

