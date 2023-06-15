/*
 * pgflt_test.c
 *  
 * this program will attempt to induce page faults - can't do it!
 *
 * $ gcc -o pgflt_test pgflt_test.c
 * $ ./pgflt_test 4              # set increment to 4; 4 * 1024 for allocation
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int loop_cnt = 35; 

int main(int argc, char *argv[]) 
{
	pid_t mypid  = getpid();
	printf( "mypid: %d\n", mypid);
	char *buffer;
	int num_byte = atoi(argv[1])*1024; 

	while (loop_cnt) { 
		buffer = malloc(num_byte);
		buffer[0] = 'a';
		buffer[num_byte-1] = 'z';
		free(buffer); 
	}
	exit(0);
}
  
