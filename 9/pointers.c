//Elijah Morris
//Pointers Program
//10-18-21

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char arg[4][32] = {
    "one",
    "two",
    "three",
    ""
};

int main(int argc, char **argv, char **envp) {
    if(argv[1][0] == 'a') {
        printf("call from execve\n");
        return 0;
    }
    if(argc == 2) {
        printf("arg1: %i\n", argv[0]);
        printf("arg2: %i\n", argv[1]);
        printf("%p\n", argv[2]);
        printf("%p\n", argv[3]);
    }
    argv[1][0] = 'a';
    execve("a.out", argv, envp);

    int a = 25;
    int b = 35;
    int *ptr;
    int *ptrb = &b;
    ptrb = &b;

    int **ptr2;
    ptr2 = &ptrb;

    int *arr = (int *)malloc(100 * sizeof(int));

    ptr = &a;
    printf("a: %i\n", *ptr);
    printf("b: %i\n", **ptr2);


    return 0;
}
