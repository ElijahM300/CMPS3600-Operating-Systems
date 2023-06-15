/*  fork1.c     
 *  demonstrate fork system call to clone a new process
 * 
 *  compile and run with strace:
 *
 *     $ gcc -o fork1 fork1.c
 *     $ strace -c -f -o log ./fork1   # counts calls, follows child, redirects
 *                                     # stderr to log 
 *
 *  let it run without interruption (12 seconds) and view log
 *
 *  Run it again with top:
 *
 *     $ ./fork1 &
 *     $ top -u {username}
 *
 * Notes: The <unistd.h> header file defines the POSIX API 
 */

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int status;
	static int nsecs = 10;

	/* duplicate yourself - fork returns PID of the child to the parent 
	   and 0 to the child or -1 to the parent on failure */

	int cpid = fork();  /* clone a new process */

	if (cpid == 0) {
		/* Yes, this is the child process now. */
		alarm(nsecs); /* set alarm to kill yourself in 10 seconds */
		pause();      /* suspend yourself until SIGALRM signal comes in */
		exit(0);      /* exit code 0 means success */ 
	} else {
		/* This is still the parent process running. */
		/* parent waits for child to die; harvests child's exit code */
		wait (&status);  
		sleep(nsecs); /* pause to view child termination in top */ 
		exit(EXIT_SUCCESS);
	}
}
