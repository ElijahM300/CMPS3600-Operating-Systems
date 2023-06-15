/*
 *  sighand2.c
 *  signal handler example #2 
 * 
 *  Demonstrates what happens when the signal handler for one signal is 
 *  interrupted by a second incoming signal. This is accomplished by hitting 
 *  CTRL-C while in the middle of a system call.
 * 
 *  Compile and execute:
 *  
 *        $ gcc -o sighand2 sighand2.c
 *        $ ./sighand2
 * 
 *  Start typing something at the prompt. Hit CTRL-C before finishing. Note 
 *  what happens. The system call fgets is interrupted and generates its own
 *  signal EINTR (interrupted system call). Some system calls can be restarted, 
 *  fgets() is one. Make the change noted, re-run and see what happens.
 *    
 *    The prototype for a signal handler is:
 * 
 *       void sigint_handler(int sig); 
 * 
 *     Read manual: man sigaction for more details on flags
 */
  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

void sigint_handler(int sig)
{
  	/* write is async-safe */
	write(1, " Ok, got a SIGINT!\n", 19);
}

int main(int argc, char *argv[], char *envp[])
{
	char s[200];
	int my_flag = 0;
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	if (argc > 1)
		my_flag = atoi(argv[1]);

	/* pick one of the following */
	if (my_flag == 0) {
   		/* do NOT restart an interrupted system call */
		sa.sa_flags = 0;
	} else {
	   	/*restart interrupted system calls in handler*/
		sa.sa_flags = SA_RESTART;
	}

	/* the mask for the handler is empty so no signals are blocked in handler */
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	printf("Enter a string:\n");
	if (fgets(s, sizeof s, stdin) == NULL)
		perror("fgets");
	else 
		printf("You entered: %s\n", s);

	return 0;
}

