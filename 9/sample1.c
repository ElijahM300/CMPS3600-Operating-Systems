/* 
 * sample1.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *arv[], char *env[])
{
	/* make sure you terminate your array of pointers with NULL 
	 * and that you pass the full pathname to the file you wish to exec */
	char *newargv[] = { "/bin/ls", "-al", ".", NULL }; 

	/* execve without a fork overlays current process with new process */
	int rc = execve(newargv[0], newargv, NULL);

	/* nothing is executed in the original process after this point unless
	 * execve failed */
	if (rc < 0) {
		perror("execve: ");
		return -1;
	}
}
