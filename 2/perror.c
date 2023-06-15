/*  /examples/week01/perror.c
 *
 * Demonstrate use of system call error handling facilities with perror
 *
 * The include file errno.h lets you call systems level error handling routines.
 * In particular, perror(), which accesses the global variable errno that holds
 * the error code of the last failed system call. See /usr/include/sys/errno.h
 * for predefined error codes. Here are a few:
 *
 *   #define EPERM  1  // not owner 
 *   #define ENOENT 2  // no such file or directory
 *   #define ESRCH  3  // no such process
 *   #define EINTR  4  // interrupted system call 
 *   #define EIO    5  // I/O error 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/file.h>

int main(void)
{
	/* demonstrate how perror works on failed calls by trying to open a
	 * file that does not exist */

	printf("\nDemonstrate perror().\n");

	/* force a file open error */
	printf("First try to open a non-existent file:\n");
	int fd = open("nofile.txt", O_WRONLY);
	if (fd == -1) {
 		/* this string prefaces the error msg */
		perror("file open error: ");
	}

	printf("Next try to open a file that does exist:\n");
  	/* open a file that does exist */
	fd = open("Makefile", O_WRONLY);
	if (fd == -1) {
	   	/* The string can be anything to help you find the error
		 * in the program code. A debugging aid for sure. */
		perror("File Open Error: ");
	} else {
		printf("Makefile opened successfully.\n");
	  	/* close the file */
		close(fd);
	} 
	return 0;
}

