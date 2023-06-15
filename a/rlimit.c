/* rlimit.c
 * demonstrate use of getrlimit(2) call 
 */

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define _GNU_SOURCE 
#define _FILE_OFFSET_BITS == 64

int main() {

	struct rlimit rlim;

	if (getrlimit(RLIMIT_NPROC,&rlim) == -1)
		return -1;

	/* rlim_cur holds soft limit */
	printf("Max threads/process (NPROC): %lld\n", (long long) rlim.rlim_cur);

	if (getrlimit(RLIMIT_AS,&rlim) == -1)
		return -1;
	if (rlim.rlim_cur == RLIM_INFINITY) 
		printf ("VM Address Space (RLIMIT_AS): No limit\n");
	else
		printf("VM Address Space (RLIMIT_AS): %lld\n",
													(long long) rlim.rlim_cur);

	if (getrlimit(RLIMIT_DATA,&rlim) == -1)
		return -1;
	if (rlim.rlim_cur == RLIM_INFINITY) 
		printf ("Data + Heap (RLIMIT_DATA): No limit\n");
	else
		printf("Data + Heap (RLIMIT_DATA): %lld\n", (long long) rlim.rlim_cur);

	if (getrlimit(RLIMIT_RSS,&rlim) == -1)
		return -1;
	if (rlim.rlim_cur == RLIM_INFINITY) 
		printf ("Resident Set (RLIMIT_RSS): No limit\n");
	else
		printf ("Resident Set (RLIMIT_RSS): %lld\n",(long long)rlim.rlim_cur);

	if (getrlimit(RLIMIT_STACK,&rlim) == -1)
		return -1;
	if (rlim.rlim_cur == RLIM_INFINITY) 
		printf ("Max Stack Size (RLIMIT_STACK): No limit\n");
	else
		printf ("Max Stack Size (RLIMIT_STACK): %lld\n",
													(long long)rlim.rlim_cur);
	exit(EXIT_SUCCESS);
}

