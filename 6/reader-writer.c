/*
 *
 *  reader-writer.c
 *
 *  Uses shared memory, threads and semaphores to implement a solution 
 *  to a reader/writer problem: one writer has exclusive use to shared 
 *  memory and one reader has exclusive use to access shared memory. 
 *
 *  This algorithm gives no preference to either readers or writers. The 
 *  calls to fib(n) are just to slow processing down so readers and writers 
 *  will jump in and out a little more slowly - not to synchronize anything.
 *  If you execute the code multiple times you will see the random behavior
 *  of who gets in and when - but no thread is given preference. 
 *
 *    $ gcc reader-writer.c -lpthread   # link in POSIX pthread library         
 *  or
 *    $ make
 *    $ top -H -u username
 *    $ ./reader-writer 
 *
 *  The algorithm below is implemented. Who gets in first is a race condition. 
 *  Since this is not producer-consumer this is OK since shared memory is 
 *  initialized before spawning. 
 *
 *       READER     rsem    wsem     WRITER
 *      ========      0      0       =======
 * RLOOP:                        WLOOP:
 *  (assume reader grabs)           wsem.op=0
 *     wsem.op=0                    rsem.op=0  
 *     rsem.op=+1     1             wsem.op=+1
 *      
 *    {critical code}             ....blocks.... 
 *     rsem.op=-1     0      1  
 *     ...blocks.....             {critical code}
 *                           0     wsem.op=-1   
 *    GOTO RLOOP                   GOTO WLOOP
 */

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#define PERMS 0666
#define READER 0 
#define WRITER 1 
#define DEBUG 0 
#define LIMIT 50 

/*  this definition is required under all UNIXs except BSD to set value  */

union semun {
	int     val;            /* value for SETVAL  */
	struct  semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */      
	u_short *array;         /* array for GETALL & SETALL   */
}; 

/* these utility functions make semaphore coding more readable. normally
 * you want to minimize function calls in systems coding and you still  
 * need to understand the low-level semaphore operations; you these if you
 * wish - otherwise just call semop() directly. 
 */
int init_my_sems(int size, int val);  
void reader_lock(int);
void writer_lock(int);
void reader_unlock(int);
void writer_unlock(int);
void removeid(int);

int fib(int n);  /* a function for some busy work */
void Writer(void *);  /*  writer thread */
void Reader(void *);  /* reader thread  */
void ctrlc_handler(int);

/* anything you want threads to see must be global */
int shmid, semid;
int *shared;      

int main (void) {

	/* set stdout to autoflush - this call is not portable but works on linux */
	setvbuf(stdout, NULL, _IONBF, 0);

	/* create an a shared memory segment - IPC_PRIVATE creates unique set_id */
	shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);

	if (shmid == -1)
		perror("shmget:");   

	/* attach to segment and get a pointer to it */
	shared = shmat(shmid, NULL, 0 );

	/* initialize mem segment */
	*shared=0; 

	/* create a semaphore pair, set both to 0 */ 
	semid = init_my_sems(2, 0); 

	/* trap for CTRL-C */
	struct sigaction sa; 
	sa.sa_handler = ctrlc_handler;
	sigfillset(&sa.sa_mask);   /* block signals while in the handler */
	sigaction(SIGINT, &sa, NULL);

	/* create one Reader and one Writer thread - both loop until LIMIT */
	pthread_t W1, R1;

	if (pthread_create( &W1, NULL,  (void *)Writer, (void *)"1" ) != 0)
		perror("Error creating thread\n");

	if (pthread_create( &R1, NULL,  (void *)Reader, (void *)"1" ) != 0)
		perror("Error creating thread\n");

	/* rejoin from threads */
	if (pthread_join(W1, NULL) != 0 )
		perror("Error rejoining from thread\n");

	if (pthread_join(R1, NULL) != 0 )
		perror("Error rejoining from thread\n");

	/* parent cleans up */
	write(1,"\n",1);
	removeid( semid);                // remove semaphore set
	shmctl( shmid, IPC_RMID, NULL);  // remove shared memory

} // end main


/* READER reads shared memory */
void Reader(void *num)
{
	int cnt = 0;
	time_t now;    
	pid_t tid;
	tid = syscall(SYS_gettid);      
	time(&now);  /* grabs system time and stuff it into struct time_t now*/ 
	if (DEBUG)
		fprintf(stderr, "create reader thread pid: %d tid: %d  %s\n",
			getpid(), tid, ctime(&now));

	/* reader thread reads */
	while (cnt < LIMIT) {
		fib(24);   /* do some busy work */ 

		reader_lock(semid);

		/* construct a string for the reader to spit out for debugging */
		char str[5];
		char buf[80] = "reader ";
		strcat(buf,(char*)num); 
		strcat(buf," reads: "); 

		/* grab shared mem */
		sprintf(str,"%d",*shared);
		strcat(buf, str); 
		strcat(buf, "\n"); 
		if (DEBUG)
			write(1,buf,strlen(buf)); 
		cnt++;

		/* display 'r' to demonstrate race condition */
		write(1,"r",1);
		fsync(1);

		reader_unlock(semid);
	}
	pthread_exit(0);
}


/* WRITER THREAD writes timestamp to shared memory */
void Writer(void *num)
{
	int cnt = 0;
	time_t now; 
	pid_t tid = syscall(SYS_gettid);      
	time(&now);  /* grabs system time and stuff it into struct time_t now*/ 
	if (DEBUG)
		fprintf(stderr, "create writer thread pid: %d tid: %d %s\n",
			getpid(), tid, ctime(&now));

	/* construct a string for the writer to spit out to stderr */
	char buf[80] = "WRITER  ";
	strcat(buf,(char*)num);
	strcat(buf," ");
	/* char c[2] = "0"; (unused) ascii value for character 0 */

	/* make the loop random when you have multiple threads */
	while (cnt < LIMIT) {
		writer_lock(semid);
		fib(24);   /* do some busy work */ 
		/* increment shared memory */
		(*shared)++;  
		if (DEBUG)
			fprintf(stdout, "Writer changed mem to: %d\n", *shared); 
		/* display W to demonstrate race condition */
		write(1, "W", 1);
		fsync(1);
		cnt++;
		writer_unlock(semid);
	}
	pthread_exit(0);
}

void ctrlc_handler(int sig)
{
	union semun dummy;      
	semctl(semid, 0, IPC_RMID, dummy);
	shmctl(shmid, IPC_RMID, NULL); 
}


/* create a semaphore set with two semaphores;  
 * initialize both to val */
int init_my_sems( int nsems, int val )
{
	int id;
	union semun initval;
	initval.val = val;

	/* create a semaphore set if one doesn't already exist
	 * of nsems and mode rw-rw-rw ; IPC_PRIVATE means construct
	 * a semaphore identifier without using ipckey -it will be unique */
	id = semget(IPC_PRIVATE, nsems, (PERMS|IPC_CREAT|IPC_EXCL) ); 
	if ( id == -1)
		printf("error creating semaphore\n"); 

	/*  set semaphore element values to incoming val */ 
	int i;
	for (i=0; i<nsems; i++) {
		if (semctl(id, i, SETVAL, initval) == -1)
			printf("error setting semaphore value\n");
	}
	/* return the semaphore id */
	return id;
}

/*  
 *  sets the operation buffer for wait and lock operations used by Reader
 *  threads. a Reader waits only if a Writer is writing, otherwise jumps
 *  in. the operation to update the number of waiting Readers and the
 *  operation to wait on a Writer are not atomic  
 */
void reader_lock(int id) 
{
	struct sembuf lock[2];  /* reader sem ops */
	lock[0].sem_num = READER;  
	lock[0].sem_op  = +1;        
	lock[0].sem_flg = SEM_UNDO;  
	lock[1].sem_num = WRITER;  
	lock[1].sem_op  = 0;        
	lock[1].sem_flg = SEM_UNDO;  

	if (semop(id, lock, 2) == -1)
		perror("reader semop");
}

/* wait for 0 on n_readers and 0 on n_writers and increment n_writer */
void writer_lock(int id) 
{
	struct sembuf lock[3];   /* 3 semaphore operations performed atomically */

	lock[0].sem_num = READER;  
	lock[0].sem_flg = SEM_UNDO;  
	lock[0].sem_op  = 0;            // wait for n_readers = 0

	lock[1].sem_num = WRITER;  
	lock[1].sem_flg = SEM_UNDO;   
	lock[1].sem_op  = 0;            // wait for n_writers = 0

	lock[2].sem_num = WRITER;  
	lock[2].sem_flg = SEM_UNDO;  
	lock[2].sem_op  = +1;           // n_writers will never be greater than 1

	if (semop (id, lock, 3) == -1)
		perror("writer lock semop: ");
}


void reader_unlock( int id )
{
	/* set operation buffer for the reader releasing semaphores */ 
	struct sembuf release[1];  /* holds one semaphore operation */

	release[0].sem_num = READER;  
	release[0].sem_flg = SEM_UNDO;  
	release[0].sem_op  = -1;     /* decrement the number of readers */           

	if (semop(id, release, 1) == -1 )
		perror("reader releasing semaphore: ");
}

void writer_unlock( int id ) 
{
	/* set operation buffer and call release operation for sem_num */ 
	struct sembuf release[1];  /* one semaphore operation to release sem */
	release[0].sem_num = WRITER;  
	release[0].sem_flg = SEM_UNDO; 
	release[0].sem_op  = -1; 

	if (semop (id, release, 1) == -1)
		perror("releasing writer semaphore:");
}

void removeid( int id ) 
{
	/* remove semaphore set */ 
	union semun initval; 
	if ( semctl(id, 0, IPC_RMID, initval) == -1 )
		printf("error removing semaphore\n");
}

/* give the threads something to do */
int fib(int n)
{
	if (n == 1 || n == 2)
		return 0;
	return fib(n-1) + fib(n-2);
}


