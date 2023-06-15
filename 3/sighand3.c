/*
 * sighand3.c  
 * demonstrate POSIX signal handling by 
 *
 *  1) Send signals from parent to child with the kill system call  
 *  2) setting up a signal handler for CTRL-C 
 *
 *          $ gcc -o sighand3 sighand3.c 
 *          $ ./sighand3
 *
 *  Hit CTRL-C right after the parent sends a SIGSTOP to the child. Note that 
 *  CTRL-C is propagated to all processes (parent and child) that are attached 
 *  to the same terminal (tty).
 *
 *  You will see responses from both the parent and the child for the same
 *  Ctrl-C. The child will re-start as soon as you hit CTRL-C, although the 
 *  parent is supposed to sleep for 5 seconds. This is because CtrlC interrupts 
 *  the parent's sleep by setting the sleep timer to 0 (the sleep call returns 
 *  the number of seconds left in the timer). To understand what happens when 
 *  you interrupt a sleep call with CTRL-C read these manpages.
 * 
 *        $ man 3 sleep     # POSIX calls are in section 3 of manpages 
 *        $ man 3 sigset    # System V IPC is in manpages section 3
 *
 *  Note: if you hit Ctrl-C while the child is suspended the child will never
 *  get the signal. Which means the parent will be waiting in its handler for
 *  the child to exit (which it never will). This is called deadlock. Don't
 *  worry about that now - just hit Ctrl-C while the child is running.
 *
 *  Sample Output without Ctrl-C:
 *  $./sighand3
 *  1
 *  2
 *  3
 *  (you see nothing for 4 seconds)
 *  4
 *  5
 *  6
 *  7
 * Child terminated by signal 15
 * Sample Output with Ctrl-c:
 * 1
 * 2
 * 3
 * ^C
 * Child: got SIGINT.
 *
 * Parent: got SIGINT.
 * child exited normally with code 33
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

void display_status(int *);
void sigint_handler(int sig);

/* these need to be global if you want the handler can see them */
int status;  
pid_t parent, cpid;

int main(void)
{
	struct sigaction sa;

	/* setup Ctrl-C handler before the fork */ 
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0; 
	sa.sa_flags = SA_RESTART; /* restart system calls if interrupted */ 
	sigfillset(&sa.sa_mask);  /* mask all signals while in the handler */

	/* register the handler  */
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}  
	parent = getpid(); 
	int counter=0;

	cpid = fork();   

	if (cpid == 0) {
		/* CHILD process */
		/* put the child in an infinite loop */
		while (1) {
			counter++;
			printf("%d\n",counter);
			sleep(1);
		}        
		exit(0);
	} else {
		/* PARENT process */
		sleep(3);
		if (kill(cpid,SIGSTOP) < 0)
			perror("\nkill:SIGSTOP\n");
		sleep(3);
		if (kill(cpid,SIGCONT) < 0) 
			perror("\nkill:SIGCONT\n");
		sleep(3);
		if ( kill(cpid,SIGTERM) < 0)
			perror("\nkill:SIGTERM\n");
		wait(&status);  
		if (WIFSIGNALED(status)) { 
			printf("Child terminated by signal %d\n", WTERMSIG(status));
		} else {
			printf("child exited normally with exit code %d\n",
														WEXITSTATUS(status));
		}
		exit (EXIT_SUCCESS);
	}
}

/* SIGINT HANDLER */
void sigint_handler(int sig)
{
	if (cpid == 0) {
		//Child process.
		write(1, "\nChild: got SIGINT. \n", 21); 
		_exit(33);  /* child terminates */
	} else {
		/* PARENT process */
		write(1, "\nParent: got SIGINT.\n", 21); 
		return;   /* parent returns to main to harvest exit code */
	}
}

void display_status( int *status)
{
	if (WIFEXITED(*status)) {
		printf("child exited, status=%d\n", WEXITSTATUS(*status));
	} else if (WIFSIGNALED(*status)) {
		printf("child killed by signal %d\n", WTERMSIG(*status));
	} else if (WIFSTOPPED(*status)) {
		printf("child stopped by signal %d\n", WSTOPSIG(*status));
	} else if (WIFCONTINUED(*status)) {
		printf("child continued\n");
	} else {
		printf("Notice: some other status.\n");
	}
}

