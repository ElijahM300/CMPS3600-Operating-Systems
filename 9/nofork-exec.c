/* filename: nofork-exec.c
 *
 *  purpose:  test of the system call execve() without a fork:
 *  execve() without a fork usurps the caller's user space; i.e., the new 
 *  program completely takes over its host - no additional code is executed 
 *
 *  The command to be executed is passed on the command line, e.g.
 *        $ ./nofork /bin/ls -lart /tmp
 *
 *  This command overlays itself on top of the nofork process:
 *      /bin/ls -lart /tmp
 *
 *  The full path to the command must be used, i.e.
 *      $ ./nofork ls -lart /tmp
 *
 *  will fail with an error because "ls" will not be found. 
 *  The overlayed process will inherit:
 *      the same PID, PPID, UID, and GID
 *      the same CWD and TTY
 *      the same UMASK
 *      the same signal mask
 *      the same open files (except those marked FD_CLOEXEC using fcntl()) 
 *
 *  Note: One difference between execve and execvp is that in execve you define 
 *  your own environment and pass that along when you call the function, and in 
 *  execvp you use a predefined environment defined in unistd.h. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int  main(int argc, char *argv[], char *envp[])
{
	int  i;

	if (argc < 2) {
		printf("Usage: ./nofork /bin/ls, -lart, /tmp\n");
		exit(0);
	}

	fprintf(stderr, "\nmain() is running from command line: \n  ");
	for (i = 0 ; i < argc ; i++) {
		fprintf(stderr, " %s", argv[i]);
	}
	fprintf(stderr, "\n\ncalling execve(\"%s\", &argv[1], envp[]);\n", argv[1]);

	/* run command given on the rest of the line as a new process using the 
	 * same environment as in: envp[] */

	if (execve(argv[1], &argv[1], envp) < 0)
		perror("\nexecve() failed: ");

	fprintf(stderr, "\n\n");

	return(0);
}
