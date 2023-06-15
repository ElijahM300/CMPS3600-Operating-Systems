/*
 * sem_get_info.c
 *
 * Demonstrate how to get info from semaphores
 *
 * $ gcc sem_get_info.c
 * $ ./a.out
 *
 * $ man semctl    # view this manpage for details on other info you can get
 */

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <string.h>  /* For strerror(3c) */
#include <errno.h>   /* For errno        */
#include <unistd.h>  /* rand(3c)         */
#include <stdio.h>
#include <stdlib.h>  /* exit(3)          */

int main (int argc, char **argv)   
{                                 
	int nsems, semid;
	char pathname[256];
	getcwd(pathname, 256);
	strcat(pathname, "/foo");  /* foo must exist in your directory */
	key_t ipckey = ftok(pathname, 22);  

	nsems = 1;  /* the number of semaphores you want in the set */
	semid = semget(ipckey, nsems, 0666 | IPC_CREAT);
	if (semid < 0) {
		printf("Error - %s\n", strerror(errno));
		_exit(1);
	}

	struct sembuf inc[1];   /* define increment semaphore operation */
	inc[0].sem_num = 0; 
	inc[0].sem_flg = SEM_UNDO; 
	inc[0].sem_op = +1;  

	struct sembuf dec[1];    /* define decrement semaphore operation */
	dec[0].sem_num = 0;
	dec[0].sem_flg = SEM_UNDO;
	dec[0].sem_op = -1;

	struct sembuf zwait[1];    /* define wait on zero semaphore operation */
	zwait[0].sem_num = 0; 
	zwait[0].sem_flg = SEM_UNDO; 
	zwait[0].sem_op = -1;   
	(void)zwait; /* zwait is unused at this timme */

	int sem_value;
	semop(semid, inc, 1);
	sem_value = semctl(semid, 0, GETVAL);
	printf("sem value: %d\n", sem_value);

	sem_value = semctl(semid, 0, GETNCNT);
	printf("waiting for pos value: %d\n", sem_value); 

	sem_value = semctl(semid, 0, GETZCNT);
	printf("waiting for zero: %d\n", sem_value); 

	sleep(rand() % 3); /* critical section, take a nap for 0-2 seconds */

	semop(semid, dec, 1);

	sleep(rand() % 3); /* Sleep 0-2 seconds */

	exit(EXIT_SUCCESS);
}

