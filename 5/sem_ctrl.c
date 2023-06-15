/* filename: /examples/week04/sem_ctrl.c
*   Demonstrate use of semctl call to get info about a semaphore. For 
*   background read manpage on semctl.
*
*   To compile and execute:
*      $ gcc -o sem_ctl sem_ctl.c
*      $ ./sem_ctl
*
*   System V semaphore structures used in the call:
*       struct sem {
*          int semval;  # semaphore value - this is a non-negative integer 
*          int sempid;  # PID of process that performed the last op on this sem
*       };
*
*       struct sembuf {    
*          unsigned short int sem_num;  # a semaphore num in the set: 0,1,2,... 
*          short int sem_op;            # the semaphore operation 
*          short int sem_flg;           # the operation flag 
*       };    
*
*    sem_flg: A combination of the familiar IPC_NOWAIT flag, which indicates 
*    if the test should return immediately or block until it passes, and 
*    SEM_UNDO, which causes the semaphore operations to be undone if the 
*    process should exit prematurely.
*
* Values for sem_op:
*
* If sem_op is 0, sem_num is tested to see if its value is 0; if so, the 
* next test runs. If sem_num is not 0, either the operation blocks until the 
* semaphore becomes 0 if IPC_NOWAIT is not set, or the rest of the tests are 
* skipped if IPC_NOWAIT is set.
*
* If sem_op is a positive integer, the value of sem_op is added to the value 
* of the semaphore.
*
* If sem_op is a negative integer and the value of the semaphore is greater 
* than or equal to the absolute value of sem_op, then the absolute value is 
* subtracted from the value of the semaphore.
*
* If sem_op is a negative integer and the value of the semaphore is less than 
* the absolute value of sem_op, then the execution of tests immediately stops 
* if IPC_NOWAIT is true, or blocks until the semaphore's value becomes greater
* than the absolute value of sem_op if false.   
*/

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <string.h> /* For strerror(3c) */
#include <errno.h> /* For errno */
#include <unistd.h> /* rand(3c) */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>  /* exit(3) */


union {
  int  val;    
  struct semid_ds *buf;   
  unsigned short  *array; 
  struct seminfo  *__buf; 
} my_semun; 

struct sembuf grab[2], release[1];  
int n_waiting;  /* num processes waiting for the semaphore */
int sem_value;  /* the value of the semaphore */
pid_t lastpid;   /* process that performed last semop */
char pathname[200];
key_t ipckey;
int semid;

struct sigaction sa;

void ctrlc_handler ( int dummy ) {
    semop(semid, release, 1); /* release the semaphore now */
    sem_value = semctl(semid, 0, GETVAL);
    lastpid = semctl(semid, 0, GETPID);
    printf("\nHandler:\n");
    printf("\tsem value = %d  pid: %d\n", sem_value, lastpid);
}
 
int main (int argc, char **argv)   
{
    /* setup a handler so that ctrl-c can release the semaphore */
    sa.sa_handler = ctrlc_handler;
    sigaction(SIGINT, &sa, NULL);
                                  
    getcwd(pathname,200);
    strcat(pathname,"/foo"); 
    ipckey = ftok(pathname, 42);  
    
    /* get a semaphore set with 1 semaphore in it (i.e., sem 0)*/ 
    int nsem = 1; 
    semid = semget(ipckey, nsem, 0666 | IPC_CREAT);
    if (semid < 0) {
          printf("Error - %s\n", strerror(errno));
          _exit(1);
    }

    /* IMPORTANT note from semget manpage:
     * The values of the semaphores in a newly created set are indeterminate.
     * (POSIX.1-2001 is explicit on this point.) Although Linux, like many
     * other implementations, initializes the semaphore values to 0, a	porta-
     * ble application cannot rely on this: it should explicitly initialize
     * the semaphores to the desired values.
     */ 

    /* setup GRAB semaphore operation; there are two steps involved; first 
     * check if value is zero; if so grab it; otherwise wait-for-zero; these 
     * ops are executed atomically */

    grab[0].sem_num = 0;  /* apply this operation to semaphore 0 in the set */
    grab[0].sem_flg = SEM_UNDO; /* release semaphore if holder dies */ 
    grab[0].sem_op = 0;  /* wait until value is zero */
    grab[1].sem_num = 0; 
    grab[1].sem_flg = SEM_UNDO;  
    grab[1].sem_op = 1;  /* increment sem value by 1 */

    /* setup RELEASE semaphore operation */
		release[0].sem_num = 0;  /* apply the operation to semaphore 0 in the set */
		release[0].sem_flg = SEM_UNDO; /* release semaphore if holder dies */ 
    release[0].sem_op = -1; /* decrement semaphore by -1 to unlock it */

    my_semun.val = 0;   /* you can also set this to any positive number */
    semctl(semid, 0, SETVAL, my_semun); 
    sem_value = semctl(semid, 0, GETVAL);
    printf("sem value = %d  pid: %d\n", sem_value, lastpid);

    /* perform grab operation */ 
    semop(semid, grab, 2);    /* 2 means perform 2 ops in the sembuf struct */
    n_waiting = semctl(semid, 0, GETZCNT); /* lets you know status of queue */ 
    sem_value = semctl(semid, 0, GETVAL);
    lastpid = semctl(semid, 0, GETPID);
    printf("sem value = %d  pid: %d\n", sem_value, lastpid);

    /* try to grab it again - this time don't wait */ 
    grab[0].sem_flg = IPC_NOWAIT;  
    if (semop(semid, grab, 2) < 0)
         printf("\ndid not grab semaphore and not waiting around for zero\n");
    sem_value = semctl(semid, 0, GETVAL);
    lastpid = semctl(semid, 0, GETPID);
    printf("sem value = %d  pid: %d\n", sem_value, lastpid);

    /* send a signal to yourself to release semaphore */
    kill(getpid(),SIGINT);

    /* remove semaphore */
    if ((semctl(semid, 0, IPC_RMID)) < 0) {
       perror("semctrl IPC_RMID");
       exit(EXIT_FAILURE);
    } 
    exit(EXIT_SUCCESS);
}
