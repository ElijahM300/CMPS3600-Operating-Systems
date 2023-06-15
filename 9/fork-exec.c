/*
 * fork-exec.c
 *
 * purpose:  test of the system call execve() with a fork
 *
 * you can use execve() in two ways - either you take over the current process 
 * or you take over a forked child; execve() is a body snatcher in either case.
 *
 * execve() with a fork does not usurp the caller's user space; i.e., the new 
 * program executes within the space of the child process 
 *
 * The command to be executed is passed on the command line, e.g.
 *
 *       $ ./fork-exec /bin/ls -lart /tmp
 *
 * The program executes this command in the child process:
 *
 *       /bin/ls -lart /tmp
 *
 * The full path to the command must be used, i.e.
 *
 *        $ ./fork-exec ls -lart /tmp
 *
 * will fail with an error because "ls" will not be found. 
 *
 * The new process (the forked child) will inherit
 *
 *     the same CWD and TTY
 *     the same UMASK
 *     the same signal mask
 *     the same open files (except those marked FD_CLOEXEC using fcntl()) 
 *
 * The new process will get a new PID, PPID, UID, and GID
 *
 * Note: One difference between execve and execvp is that in execve you define 
 * your own environment and pass that along when you call the function, and in 
 * execvp you use a predefined environment defined in unistd.h. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int status;

int main(int argc, char *argv[], char *envp[])
{
	int  i;

	if (argc < 2) {
		printf("Usage: ./fork-exec /bin/ls, -lart, /tmp\n");
		exit(0);
	}

	fprintf(stderr, "\nmain() is running from command line: \n  ");
	for (i=0; i<argc; i++) {
		fprintf(stderr, " %s", argv[i]);
	}
	fprintf(stderr, "\n\n");
	fprintf(stderr, "\n\ncalling execve(\"%s\", &argv[1], envp[]);\n", argv[1]);

	/* run command given on the rest of the line as a new process using the 
	 * same environment as in: envp[] */

	/* FORK and EXEC */
	int child = fork();
	if (child == 0) {
		/* child process */
		if (execve(argv[1], &argv[1], envp) < 0)
			perror("\nexecve() failed: ");
	} else {
		wait(&status);
		printf("exec'd child exited on status: %d\n", status);
	}
	return 0;
}

