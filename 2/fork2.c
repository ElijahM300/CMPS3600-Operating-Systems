/*
 * fork2.c
 *      
 * demonstrate that child
 *     1) inherits data values but does not share data after fork
 *     2) child and parent share open file descriptors after fork
 * 
 *  compile, execute and view log file:
 *
 *    $ gcc -o fork2 fork2.c
 *    $ ./fork2  
 *    $ cat log
 */

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int glob_var = 100;           /* global with external linkage */
static int static_var  = 50;  /* global with internal linkage */

int main(void)
{
	int status;
	char buf[100];
	pid_t cpid, parent;
	int logfd;         /* parent and child share this file descriptor */
	int loc_var = 25;  /* stack dynamic local */

	parent = getpid();
	(void)parent; /* to avoid a compile warning */

	/* open file for writing only, create if it does not exist. */
	/* truncate file if it does exist.                          */
	/* open with 644 permissions                                */
	logfd = open("log", O_WRONLY|O_CREAT|O_TRUNC, 0644);

	/* write values before fork to screen and log */
	sprintf(buf, "before fork: %d, %d, %d\n", glob_var, static_var, loc_var);
	write(1, buf, strlen(buf));
	write(logfd, buf, strlen(buf));

	/* fork returns pid of child to the parent, 0 to child */
	cpid = fork();

	if (cpid == 0) {
		/* 0 means you are the child */
		/* Yes, this is the child */

		/* write values in child after fork to screen and log */
		sprintf(buf,"child after fork: %d, %d, %d\n",
												glob_var, static_var, loc_var); 
		write(1, buf, strlen(buf));
		write(logfd, buf, strlen(buf));

		glob_var = 15;
		static_var =  45;
		loc_var =  60;

		/* write modified values to screen and log */
		sprintf(buf,"child new values: %d, %d, %d\n",
												glob_var, static_var, loc_var); 
		write(1, buf, strlen(buf));
		write(logfd, buf, strlen(buf));

		/* child sets an alarm to go off in 5 seconds */
		alarm(5); 

		/* child loops until alarm signals it to terminate */
		while (1) { }

		exit(0); 

	} else {
		/* This is still the parent process. */

		wait(&status);   /* wait for the child to die */

		sprintf(buf, "child terminated with exit code %d\n", status);
		write(logfd, buf, strlen(buf));

		/* write values to screen and log after child dies */
		sprintf(buf, "parent after wait: %d, %d, %d\n",
												glob_var, static_var, loc_var); 
		write(1, buf, strlen(buf));
		write(logfd, buf, strlen(buf));

		/* parent can safely close file now */
		close(logfd);
		exit (EXIT_SUCCESS);
	}
}
