/*  
 *  strong-writer.c   (AKA weak readers or writers preference)
 *
 *  readers are almost effectively starved! if you run it 4-5 times you
 *  will see a reader getting the chance to jump in.
 *  In a strong writer algorithm preference is given to waiting writers; i.e.,
 *  Any writer waiting will be allowed to finish writing before any reader is 
 *  allowed to read. This is implemented by grab the number of writers that 
 *  are waiting:
 *
 *            int n_writers_waiting = semctl(semid, READER, GETZCNT);
 *
 *    $ gcc strong-writer.c -lpthread   # link in POSIX pthread library         
 *  or
 *    $ make
 *    $ top -H -u username
 *    $ ./strong-writer  2>err
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
#define DEBUG 1 

/*  this definition is required under all UNIXs except BSD to set value  */
union semun {
	int     val;            /* value for SETVAL  */
	struct  semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */      
	u_short *array;         /* array for GETALL & SETALL   */
}; 

/* these utility functions make semaphore coding more readable */
int init_my_sems(int size, int val);  
void reader_lock(int);
void writer_lock(int);
void reader_unlock(int);
void writer_unlock(int);
void removeid(int);

/* busy work */
int fib(int n);  

/* thread functions */ 
void Writer(void *); 
void Reader(void *);  

void ctrlc_handler(int);

int seg_id, semid;
int *shared;      

int main( void ) {

	/* set stdout to autoflush - this call is not portable but works on linux */
	setvbuf(stdout, NULL, _IONBF, 0);

	/* create an a shared memory segment */
	seg_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);

	if (seg_id == -1)
		perror("shmget:");   

	/* attach to segment and get a pointer to it */
	shared = shmat(seg_id, NULL, 0 );

	/* initialize mem segment */
	*shared=0; 

	/* create a semaphore pair, set both to 0 */ 
	semid = init_my_sems(2, 0); 

	/* trap for CTRL-C */
	struct sigaction sa; 
	sa.sa_handler = ctrlc_handler;
	sigfillset(&sa.sa_mask);   /* block signals while in the handler */
	sigaction(SIGINT, &sa, NULL);

	/* create Reader and Writer threads. The Reader thread performs a 
	 * loop of read operations, reading only when the Writer has finished. 
	 * demonstrates the strong-writer preference protocol; e.g. the reader must 
	 * wait until no writers are waiting
	 */
	pthread_t W1, W2, W3, R1, R2, R3, R4;
	if (pthread_create( &R1, NULL,  (void *)Reader, (void *)"1") != 0)
		printf("Error creating thread\n");
	if (pthread_create( &R2, NULL,  (void *)Reader, (void *)"2") != 0)
		printf("Error creating thread\n");
	if (pthread_create( &R3, NULL,  (void *)Reader, (void *)"3") != 0)
		printf("Error creating thread\n");
	if (pthread_create( &R4, NULL,  (void *)Reader, (void *)"4") != 0)
		printf("Error creating thread\n");

	if (pthread_create( &W1, NULL,  (void *)Writer, (void *)"1") != 0)
		printf("Error creating thread\n");
	if (pthread_create( &W2, NULL,  (void *)Writer, (void *)"2") != 0)
		printf("Error creating thread\n");
	if (pthread_create( &W3, NULL,  (void *)Writer, (void *)"3") != 0)
		printf("Error creating thread\n");

	/* rejoin from threads */
	if (pthread_join(W1, NULL) != 0)
		fprintf(stderr,"Error rejoining from thread\n");
	if (pthread_join(W2, NULL) != 0)
		fprintf(stderr,"Error rejoining from thread\n");
	if (pthread_join(W3, NULL) != 0)
		fprintf(stderr,"Error rejoining from thread\n");
	if (pthread_join(R1, NULL) != 0)
		fprintf(stderr,"Error rejoining from thread\n");
	if (pthread_join(R2, NULL) != 0)
		fprintf(stderr,"Error rejoining from thread\n");
	if (pthread_join(R3, NULL) != 0) 
		fprintf(stderr,"Error rejoining from thread\n");
	if (pthread_join(R4, NULL) != 0) 
		fprintf(stderr,"Error rejoining from thread\n");

	/* parent cleans up */
	removeid(semid);                  // remove semaphore set
	shmctl( seg_id, IPC_RMID, NULL);  // remove shared memory

} // END MAIN 


/* READER THREAD reads from shared memory and does some busy work */
void Reader( void * num)
{
	int id = atoi((char *)num);
	int n_writers_waiting = 0;  /* add this to implement writers preference */
	time_t now = time(NULL);   /* always initialize */   
	int i;
	char str[5];
	pid_t tid;
	tid = syscall(SYS_gettid);      
	time(&now);  /* grabs system time and stuff it into struct time_t now*/ 
	if (DEBUG)
		fprintf(stderr,"create reader thread pid: %d tid: %d  %s\n",
			getpid(), tid, ctime(&now));

	/* each reader thread simply reads three times then exits */
	for (i=1; i<=3; i++) {

		fib(25);   /* do some busy work */ 

		/* check writers waiting on readers */
		n_writers_waiting = semctl(semid, READER, GETZCNT); 

		if (!n_writers_waiting) {    
			reader_lock(semid);
			fib(30);   /* do some busy work */ 

			/* construct a string for the reader to spit out */
			char buf[80] = "READER ";
			strcat(buf, (char*)num); 
			strcat(buf, " READS "); 

			/* grab shared mem and display it */
			sprintf(str, "%d", *shared);
			strcat(buf, str); 
			strcat(buf, "!\n"); 
			write(1, buf, strlen(buf)); 
			fsync(1);

			reader_unlock(semid);
		}
		else
			fprintf(stdout,"reader %d sees %d writers waiting.\n",
				id,n_writers_waiting);
	}
	pthread_exit(0);
}


/* WRITER THREAD writes timestamp to shared memory */
void Writer(void* num)
{
	time_t now; 
	int i;
	pid_t tid = syscall(SYS_gettid);      
	time(&now);  /* grabs system time and stuff it into struct time_t now*/ 
	if (DEBUG)
		fprintf(stderr,"create writer thread pid: %d tid: %d %s\n",
			getpid(), tid, ctime(&now));

	/* construct a string for the writer to spit out to stderr */
	char buf[80] = "WRITER ";
	strcat(buf, (char*)num); 
	strcat(buf, " WRITES MEMORY.\n"); 
	/*char c[2] = "0"; (unused) */ /* ascii value for character 0 */


	for (i=1; i<= 3; i++){
		writer_lock(semid);

		fib(20+(2*i));   /* do some busy work to queue up some readers */ 

		/* increment shared memory */
		(*shared)++;

		write(1, buf, strlen(buf));
		//fprintf(stdout,"Writer changed mem to: %d\n",*shared); 
		writer_unlock(semid);  
	}
	pthread_exit(0);
}

void ctrlc_handler(int sig)
{
	union semun dummy;      
	semctl(semid, 0, IPC_RMID, dummy);
	shmctl(seg_id, IPC_RMID, NULL); 
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

/* sets the operation buffer for wait and lock operations used by Reader
 * threads. a Reader waits only if a Writer is writing, otherwise jumps
 * in. the operation to update the number of waiting Readers and the
 * operation to wait on a Writer are not atomic  
 */
void reader_lock(int id) 
{
	struct sembuf lock[2];  /* reader sem ops */
	lock[0].sem_num = WRITER;  
	lock[0].sem_op  = 0;        /* wait until there are zero writers */
	lock[0].sem_flg = SEM_UNDO;  
	lock[1].sem_num = READER;  
	lock[1].sem_op  = +1;        
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


/* set operation buffer for the reader releasing semaphores */ 
void reader_unlock(int id) 
{
	struct sembuf release[1];  /* holds one semaphore operation */

	release[0].sem_num = READER;  
	release[0].sem_flg = SEM_UNDO;  
	release[0].sem_op  = -1;     /* decrement the number of readers */           
	if (semop(id, release, 1) == -1)
		perror("reader releasing semaphore: ");
}

/* set operation buffer and call release operation for sem_num */ 
void writer_unlock(int id)
{
	struct sembuf release[1];  /* one semaphore operation to release sem */
	release[0].sem_num = WRITER;  
	release[0].sem_flg = SEM_UNDO; 
	release[0].sem_op  = -1; 

	if (semop (id, release, 1) == -1)
		perror("releasing writer semaphore:");
}

/* remove semaphore set */ 
void removeid( int id )
{
	union semun initval; 
	if (semctl(id, 0, IPC_RMID, initval) == -1)
		printf("error removing semaphore\n");
}

/* give the threads something to do */
int fib(int n) {
	if (n==0) return 0;
	else
		if (n==1) return 1;
		else
			return fib(n-1) + fib(n-2);
}
