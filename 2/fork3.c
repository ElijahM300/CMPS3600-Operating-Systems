/*
    fork3.c     

    Demonstrate how a child becomes a zombie; if the child dies before its 
    parent the child is a zombie (aka defunct process) until the parent 
    harvests the child's exit code. A zombie takes up an entry in the kernel's
    process table so leaving a zombie should be avoided. 

    Compile and run with strace, logging stderr to errfile:
  
      $ gcc -o fork3 fork3.c
      $ strace -q -f ./fork3  2> errfile 
  
    Let it run in background, run top and view the outfile.
  
       $ ./fork3 6 >out &
       $ top -u {username}    # note the 'Z' when the child dies
       $ cat out log
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/file.h>  
#include <sys/types.h>  
#include <sys/wait.h>  
#include <unistd.h>     /* header file for the POSIX API */

#define BUFSZ 100

/* do this before using buffer for write calls */
void zeroOutBuf(char *str)
{
	int i;
	for (i=0; i<BUFSZ; i++)
		str[i] = 0;
}

int main(int argc, char *argv[])
{
	int status = 0;
	pid_t parent, cpid;  /* pid_t is POSIX standard for system API */
	//int i;
	int nsecs = 0;

	/* grab nsecs if passed */
	if (argc == 2) {
		nsecs = atoi(argv[1]);
	} else {
		nsecs = 0;
	} 

	/* grab parent's pid */
	parent = getpid(); 

	/* sleep long enough to see parent process alone running */
	sleep(8);

	/* FORK */
	cpid = fork();  
	if (cpid < 0) {
		perror("fork: ");
		exit(0);
	}
	if (cpid == 0) { 
		/* CHILD process */ 

		int exitcode = 77;
		char buf[BUFSZ];
		zeroOutBuf(buf);

		/* open a log file */
		int logf;
		logf = open("log", O_WRONLY|O_CREAT|O_TRUNC, 0644);  
		if (logf == -1) 
			perror("File open error: "); 

		/* write info to child's log file */
		time_t timer;
		struct tm *tm_info;
		time(&timer);
		tm_info = localtime(&timer);
		strftime(buf, 26, "%Y:%m:%d %H:%M:%S\n", tm_info);
		write(logf,buf,strlen(buf));

		sprintf(buf,"child's pid: %d\n",getpid()); 
		write(logf,buf,strlen(buf));

		sprintf(buf,"child's exit code: %d\n", exitcode);
		write(logf,buf,strlen(buf));

		close(logf);  

		sleep(2);

		exit(exitcode); 
	} else {
		/* PARENT process */
		char buf[BUFSZ];
		sprintf(buf,"from parent. pid: %d, status: %d\n", parent, status);
		write(1, buf, strlen(buf));
		sleep(nsecs);  /* suspend parent process to see zombie */
		wait(&status);
		sleep(nsecs);  /* suspend parent process again to see zombie gone */
		if (WIFEXITED(status))
			sprintf(buf,"my child exited with code: %d\n", WEXITSTATUS(status));
		write(1,buf,strlen(buf));

		exit (EXIT_SUCCESS);
	}
}

