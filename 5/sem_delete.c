/*
 * sem_delete.c
 *
 *  Demonstrate removing semaphores. You can also remove by:
 *
 *   $ ipcs
 *   $ ipcrm -s {semaphore id}  
 *
 *  The process that creates the semaphore set should remove it.
 *  If you don't know which process will finish first, you need to
 *  remove the semaphore after both processes finish.
 *
 */ 

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h> /* For strerror(3c) */
#include <errno.h>  /* For errno */
#include <stdio.h>
#include <stdlib.h>  /* exit(3) */

int main (int argc, char *argv[]) 
{               
	/* a semaphore identifier comes into this program on the command-line */

	int semid;

	if (argc == 2) {
		/* consider the second argument to be the id */
		semid = atoi(argv[1]);
		printf("semaphore id: %d\n", semid);
	} else {
		printf("USAGE: sem_rem <semaphore id>\n");
		exit(EXIT_FAILURE);
	}

	/* now remove the semaphore */

	if ((semctl(semid, 0, IPC_RMID)) < 0) {
		perror("semctrl IPC_RMID");
		exit(EXIT_FAILURE);
	} else {
		puts("semaphore removed");
		system("ipcs -s");
	} 

	exit(EXIT_SUCCESS);
}

