#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char *chunk = (char *)malloc(40000000);

    char *mem = chunk;
    int offset = 0;
    int *a, *b;
    a = (int *)(mem + offset);
    offset += sizeof(int);
    *a = 25;
    if((unsigned long)a != (unsigned long)mem)
        printf("Error: %p %p\n", a, mem);
    printf("*a: %i   a: %p\n", *a, a);
    b = (int *)(mem + offset);
    offset += sizeof(int);
    *b = 35;
    printf("*b: %i  b: %p\n", *b, b);

    if(chunk) {
        free(chunk);
    }

	printf("Hello World\n");
	return 0;
}
