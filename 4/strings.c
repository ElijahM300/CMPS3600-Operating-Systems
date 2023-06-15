/* filename: strings.c

   demonstrate string facilities in C; also show user space for a process
   looks like in terms of stack, code, data, and the heap.  

   Pay particular attention to the memory addresses being printed out of
   certain variables throughout the code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stuff();

int MAX = 7; 
int int_array[50000000];    /* Large array to take up a lot of memory */

int main()
{
	int y = 6;

	printf("function adress main is: %p\n", main);
	printf("function address stuff is: %p\n", stuff);

	/* addresses for globals are in the data segment of the executable */
	printf("global &MAX is : %p\n",&MAX);
	printf("global &int_array of size 300 : %p\n",&int_array);

	/* if you change the array size, the size of the executable changes -
	   the user stack grows downwards from a high address to a low address */
	printf("main: &int is: %p\n\n",&y);
	stuff();
	return 0;
}

void stuff ()
{
	/* GNU C supports heap dynamic and stack dynamic arrays */
	int n = 0;
	char junk[10];

	do {
		printf("\nEnter a number > 100: ");
		while(!scanf ("%d",&n)) {   /* input validation for getting a number */
			printf("\n You must enter a number: ");
			scanf("%10s", junk);
		}
		scanf("%c", junk); /* get rid of the trailing return */
		/* Validate that we got a large enough number */
		if(n <= 100) printf("Number must be above 100!\n");
	} while (n <= 100);

	printf("\nAllocating a dynamic string of size %ld.\n", sizeof(char[n]));
	char * dstr = (char *) malloc(sizeof(char[n]));  /* dynamic string */

	/* demonstrate how to read strings in from the keyboard (stdin) */
	char fstr[81];  /* fixed string with room for \0 */
	printf("\nEnter a sentence (enter 0-9 to end input): ");
	/* Limit scanf to 80 characters as that's the max that fstr can hold
	 * and use regex to specify what to read (letters and spaces) */
	scanf ("%80[a-zA-Z ]", fstr);
	printf("You just input: %s\n", fstr);

	/* Use strncpy for safe copy. Otherwise a buffer overflow could occur if
	 * n is < 81 */
	strncpy(dstr,fstr,n-1); /* dstr is destination and fstr is source */

	/* Create an integer pointer and allocate one integer to it */
	int * x = (int *) malloc(sizeof(int));
	int stack_arr[n]; 

	/* demonstrate that arrays are "limited" dynamic length in C
	   and not allocated exactly the number of bytes specified  */ 
	int start = 0;    // just to mark address
	char str2[10] = "hi";
	char str3[20] = "hello";
	char str4[30] = "hello goodbye";

	printf("stack frame of stuff\n");
	printf("stack dynamic array : %p\n",stack_arr);
	printf("&start is   : %p\n",&start);
	printf("&str2[10] is: %p\n",str2);
	printf("&str3[20] is: %p\n",str3);
	printf("&str4[30] is: %p\n\n",str4);

	/* The heap grows up from a low memory address to high address */
	printf("heap dynamic &dstr[n]: %p\n",dstr);
	printf("heap dynamic &x     : %p\n\n",x);

	/* Deallocate the memory allocated with malloc() */
	free(x);
	free(dstr);
}

