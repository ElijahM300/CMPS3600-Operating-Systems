/*  alarm.c 
 *  demonstrate sending SIGALRM to yourself and setting up a handler for it
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

void sigalrm_handler(int sig);

int	main(int argc, char *argv[], char *envp[])
{
	int nsecs = 1;
	if (argc > 1)
		nsecs = atoi(argv[1]);

	char buf[200]; /* use this to construct strings for write syscall */

	/* setup a signal handler for SIGALRM */
	struct sigaction sa;
	sa.sa_handler = sigalrm_handler;
	/* if you want to restart your system call uncomment the line below */
	/* sa.sa_flags = 0;  */
	sa.sa_flags = SA_RESTART; 
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGALRM, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	alarm(nsecs);  /* set timer to deliver SIGALRM to yourself in 5 secs; by 
					* default terminates the process - we setup a handler to
					* trap SIGALRM and not terminate */

	/* continuous looping until the kernel delivers the SIGALRM signal */
	while (1) {
		usleep(100000); /* delay for 1/10 seconds */
		write(1, "+", 1);
	}

	/* The code below should never execute because handler exits the program */
	strcpy(buf, "This message should never be printed.\n");
	write(1, buf, strlen(buf));

	return(0);
}

void sigalrm_handler(int sig)
{
	/* write is async-safe - printf is not */
	write(1, "\nGot SIGALRM\n", 13);

	/* if you remove the exit below you will return to main and continue 
	   executing code where signal occurred - the infinite loop */
	_exit(0); 
}

