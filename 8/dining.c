/*
 *  dining.c
 *
 *  a solution to the dining philosophers problem using mutexes 
 *
 *  code uses the C ternary conditional operator '?', which is
 *  a shorthand if-else statement: n = (flag==1) ? 10 : 20;  
 *  meaning: if flag is 1 then n is assigned value 10 otherwise value 20.
 *
 *   ./dining     // runs by quickly
 *   ./dining 40  // slows things down to view kernel thread scheduling
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LIMIT 5   /* this is the number of times each philosopher can eat */

/* this structure will be passed to each thread function */ 
typedef struct philData {
	int philno;  /* philosopher number */
	pthread_mutex_t *forklft, *forkrgt;
	const char *name;
	pthread_t thread_id;
	int fail;
} Philosopher;

const char *nameList[] = {"Kant", "Hegel", "Plato", "Aristotle", "Decartes"};

/* count times each philosopher eats to verify no starvation */
int cnt[5] = {0,0,0,0,0};

int n = 1; /* pass this value to fib */
void *philfunc(void *p);
int fib(int n);


int main(int argc, char *argv[])
{
	int i,failed;
	if (argc == 2)
		n=atoi(argv[1]);

	/* initialize count array */
	/*for (i = 0; i<5; i++) 
		cnt[0] = 0;*/

	pthread_mutex_t forks[5];
	Philosopher philptrs[5];
	Philosopher *phil;

	for (i=0; i<5; i++) {
		/* initialize the mutexes - NULL flag means the defaults are used */
		failed = pthread_mutex_init(&forks[i], NULL);
		if (failed) {
			printf("Failed to initialize mutexes.");
			exit(1);
		}
	}
	for (i=0; i<5; i++) {
		phil = &philptrs[i];
		phil->name = nameList[i];
		phil->philno = i;
		phil->forklft = &forks[i];
		phil->forkrgt = &forks[(i+1) % 5];
		phil->fail = pthread_create( &phil->thread_id, NULL, philfunc, phil);
	}
	/* join all the threads with main thread */ 
	for(i=0; i<5; i++) {
		phil = &philptrs[i];
		if (pthread_join(phil->thread_id, NULL) < 0) {
			printf("error joining thread for %s", phil->name);
		} else {
			printf("joined %s who ate %d times.\n",
												phil->name, cnt[phil->philno]);
		}
	}
	exit(0);
}

void *philfunc(void *p)
{
	/* thread function */ 
	int eat_cnt, failed, tries_left;
	int turn_cnt = LIMIT;  /* number of times each phil gets to eat */
	Philosopher *phil = p;
	pthread_mutex_t *forklft, *forkrgt, *forktmp;

	while (turn_cnt > 0) {
	 	/* make n large if you want to view kernel thread scheduling */
		fib(n);
		forklft = phil->forklft;
		forkrgt = phil->forkrgt;
		tries_left = 2;   /* try twice before being forceful */
		do {
			/* block on left fork grab */
			failed = pthread_mutex_lock(forklft);
			/* if you made it here you know you have the left fork
			 * so try right fork */

			/* mutex_trylock() returns zero if lock acquired else error code;
			 * mutex_trylock does not block -
			 * mutex_lock blocks and returns id of lock owner.
			 * implements try twice before forcefully grabbing it 
			 */ 
			failed = (tries_left>0) ? pthread_mutex_trylock(forkrgt)
			                        : pthread_mutex_lock(forkrgt);

			if (failed) {
				pthread_mutex_unlock(forklft);
				forktmp = forklft;
				forklft = forkrgt;
				forkrgt = forktmp;
				tries_left -= 1;
			}
		} while(failed && turn_cnt);

		if (!failed) {
			cnt[phil->philno]++; 
			eat_cnt = cnt[phil->philno];
			printf("%9s %6hu taking turn %d\n",
									phil->name, (int)phil->thread_id, eat_cnt);
			fflush(stdout);
			pthread_mutex_unlock(forkrgt);
			pthread_mutex_unlock(forklft);
			turn_cnt--;
		}
	}
	pthread_exit(0);
}

int fib(int n)
{
	if (n == 1 || n == 2)
		return 1;
	return fib(n-1) + fib(n-2);
}

