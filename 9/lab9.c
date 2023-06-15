/* Elijah Morris
 *  lab9.c    CMPS 3600 LAB-9 
 *  5 threads compete for a shared resource 
 *  modify to add fork/execve calls
 * 
 *  After re-joining all threads in main either do a fork followed by an exec 
 *  or an exec without a fork to cleanup semaphores 
 *
 *  Usage:
 *  ./lab9 0   # code this to exec ipcrm
 *  ./lab9 1   # code this to fork & exec ipcrm
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BATON 0 
#define DEBUG 0

union semun {
	int val;                 /* value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
} my_semun;

const char *nameList[]={"t1","t2","t3","t4","t5"};

typedef struct thrData {
	const char *name;
	pthread_t thread;
	int num;
	int rc;   /* holds the return code from pthread_create */
} Runner;

/* each thread has its own baton grab and release operation */
struct sembuf baton_grab[5][1];
struct sembuf baton_release[5][1];

int MAX_TURNS = 1;   /* all 5 threads have MAX_TURNS turns at grabbing baton -
					  * in first round sequential order is imposed - after 
					  * that grabbing would be a free-for-all */

static int semid = 0, status;
int nsems = 1;
pid_t cpid;

void *thrfunc(void *p);

int main(int argc, char* argv[], char* envp[])
{
	int i;

	int fork_flag = 0;  /* 0 means exec without fork; 1 means fork then exec */

	if (argc < 2) {
		printf("Usage: ./lab9 flg (0 means no fork and 1 means fork)\n");
		exit(0);
	}
	fork_flag = atoi(argv[1]);

	char pathname[200];
	getcwd(pathname,200);
	strcat(pathname,"/foo");  /* foo must exist in current directory */
	key_t ipckey = ftok(pathname, 45);  
	if (ipckey < 0) {
		printf("Error creating ipckey\n");
		exit(-1);
	}

	/* setup semaphore operation */
	for (i = 0; i < 5; i++) {
		baton_grab[i][0].sem_num = BATON;
		baton_grab[i][0].sem_op = -(i+1);
		baton_grab[i][0].sem_flg = SEM_UNDO;

		baton_release[i][0].sem_num = BATON;
		baton_release[i][0].sem_op = (i==4) ? 1 : +(i+2);
		baton_release[i][0].sem_flg = SEM_UNDO;
	}

	/* create one semaphore */
	if ((semid = semget(ipckey, nsems, 0666 | IPC_CREAT)) < 0) {
		perror("Error creating semaphores ");
		exit(EXIT_FAILURE);
	} else {
		if (DEBUG)
			printf("Semid: %d \n",semid);
	}

	/* initialize value of semaphore to 1 to let only first thread in */  
	my_semun.val = 1;
	semctl(semid, BATON, SETVAL, my_semun);

	/* create array of structures to be passed to each thread */
	Runner thrds[5];
	Runner *thr;

	/* create threads in reverse order to demonstrate semaphore is working */
	for (i=4; i>=0; i--) {
		thr = &thrds[i];
		thr->name = nameList[i];
		thr->num = i;
		thr->rc = pthread_create( &thr->thread, NULL, thrfunc, thr);
		printf("created %d\n",i);
		fflush(stdout);
	}

	/* rejoin all threads */ 
	for (i=0; i<5; i++) {
		thr = &thrds[i];
		if (!thr->rc && pthread_join(thr->thread, NULL)) {
			printf("error joining thread for %s", thr->name);
			exit(1);
		} else {
			printf("\njoined %s", thr->name);
		}
	}
	printf("\n");

	/* add the code for the fork and/or exec goes here - see lab instructions 
	 * since it is a little messy 
	 */
    char semid_str[20];
    sprintf(semid_str, "%d%c", semid, '\0');
    char *newargv[] = {"/usr/bin/ipcrm", "-s", semid_str, NULL};
    char *newenv[] = {NULL};

    if(fork_flag == 0) {
        printf("Program calling execve to cleanup sems...\n");
        execve(newargv[0], newargv, newenv);
    }
    else if(fork_flag == 1) {
        printf("Program is forking...\n");
        cpid = fork();
    }

    if(cpid == 0) {
        //child process
        printf("Child calling execve to cleanup sems...\n");
        execve(newargv[0], newargv, newenv);
    } 
    else {
        //parent process
        wait(&status);
        printf("Parent waited and now exiting\n");
        exit(0);
    }

	return 0;
}


/* THREAD FUNCTION */
void *thrfunc(void *p)
{
	Runner *thr = p;
	int turns = 1;
	int tnum = thr->num;

	while (turns <= MAX_TURNS) {

		/* try to grab baton - only first thr can get it ; first thread then
		 * releases second thread - and so on */

		semop(semid, baton_grab[tnum], 1);
		printf("[%d]", tnum);
		fflush(stdout); 

		/* release next thread */
		semop(semid, baton_release[tnum], 1); 
		turns++;
	}
    return (void *)NULL;
}


