/*
 * cache_check.c
 *
 * cache_size ranges from 2^10 * 4  to 2^20 * 4  (4K to 4MB given 4-byte int)
 * 1. allocate one cache_array of size 4MB
 * 2. for each of 11 cache_sizes ranging from 4K - 4MB (chunks of the one array)
 * 3. for each of stride_sizes 2^0, 2^1, 2^2, ... 2^cache_size/2  
 * 4. access cache_array by stride_size 
 *   (note: reduce cache_size by stride_size to prevent out-of-range access)
 * 5. subtract loop overhead
 *
 */

#include <stdio.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define CACHE_MIN (1024)        /* smallest cache */
#define CACHE_MAX (1024*1024*2) /* largest cache 2^21 */
#define SAMPLE 10               /* to get a larger time sample */
#ifndef CLK_TCK
#define CLK_TCK 60              /* number clock ticker per second */
#endif
int x[CACHE_MAX];               /* the array we are striding through */

double get_seconds(void)
{
	/* routine to read time */
	struct tms rusage;
	times(&rusage); /* UNIX utility: time in clock ticks */
	return (double)(rusage.tms_utime)/CLK_TCK;
}

int main(void)
{
	int register i, index, stride, limit, temp;
	int steps, tsteps, csize;

	printf("pid: %d\n", getpid());

	double sec0, sec; /* timing variables */
	for (csize=CACHE_MIN; csize<=CACHE_MAX; csize=csize*2) {
		for (stride=1; stride<=csize/2; stride=stride*2) {
			sec = 0;                    /* initialize timer */
			limit = csize-stride+1;     /* cache size this loop */
			steps = 0;
			do {
				/* repeat until collect 1 second */
			 	/* start a timer */ 
				sec0 = get_seconds();
				/* larger sample */
				for (i=SAMPLE*stride; i!=0; i=i-1) {
					for (index=0; index<limit; index=index+stride) {
						/* cache access */
						x[index] = x[index] + 1;
					}
				}
	  			/* count of loop iterations */
				steps = steps + 1;
			   	/* end the timer */
				sec = sec + (get_seconds() - sec0);
			} while (sec < 1.0);

			/* repeat empty loop to subtract loop overhead from the timer */
		   	/* used to match no. while iterations */
			tsteps = 0;
			do {
				sec0 = get_seconds();
				for (i=SAMPLE*stride; i!=0; i=i-1)
					for (index=0; index<limit; index=index+stride)
						temp = temp + index;
				tsteps = tsteps + 1;
				sec = sec - (get_seconds() - sec0);
			} while (tsteps < steps);

			printf("size:%7ld Stride:%7ld read+write:%14.0f ns\n",
				csize*sizeof(int),
				stride*sizeof(int),
				(double)sec*1e9/(steps*SAMPLE*stride*((limit-1)/stride+1)));
		}
	}
	return 0;
} 

