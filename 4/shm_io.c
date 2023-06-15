/* shm_io.c
 *
 * Demonstate writing and reading from SysV IPC shared memory 
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define SIZE 256

int main(void) {

	int shmid;
	char pathname[128];
	key_t ipckey;

	/* this allows you to generate a pathname on the fly */
	getcwd(pathname,128);
	strcat(pathname,"/foo");    /* make sure file foo is in your directory*/
	printf("%s \n",pathname);
	ipckey = ftok(pathname, 12);

	shmid = shmget(ipckey, SIZE, IPC_CREAT | 0666);
	if ( shmid < 0 ) {
		perror("shmget");
		exit(EXIT_FAILURE);
	}

	char * shared;
	shared = shmat(shmid, (void *) 0, 0);
	if ( shared < 0 ) {
		perror("shmat");
		exit(EXIT_FAILURE);
	}

	char in_name[SIZE]; 
	char out_name[SIZE]; 
	strcpy(in_name,"Samantha\n");
	write(1, in_name, 10); 
	fflush(stdout);

	strcpy(shared, in_name); 
	write(1, shared, 10); 
	strcpy(out_name, shared); 
	fflush(stdout);

	shmdt(shared);  /* detach from shared mem */
	shmctl(shmid, IPC_RMID, 0);  /* remove the segment */

	/* create another shared memory segment of an integer - 4 bytes */
	shmid = shmget(ipckey, sizeof(int), IPC_CREAT | 0666);
	int * shared_int;
	shared_int = shmat(shmid, (void *) 0, 0);

	*shared_int = 6789;  /* write to shared memory */
	printf("number: %d\n",*shared_int);   /* print to a string */
	char out_string[15];
	sprintf(out_string, "%d",*shared_int);
	printf("itoa: %s\n",out_string);

	shmdt(shared_int);  /* detach from shared mem */
	shmctl(shmid, IPC_RMID, 0);  /* remove the segment */

	return 0;

}
