/* Elijah Morris
 * Lab 2
 * 8-31-21
 */

#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int fib(int n){
    if(n == 1 || n == 2)
        return n;
    else
        return fib(n - 1) + fib(n - 2);
}

int main(int argc, char *argv[]){
    int n = 0;
    int status = 0;
    if(argc == 2) {
        n = atoi(argv[1]);
    }
    else {
        n = 0;
        printf("Usage: lab 2 n\n");
        exit(1);
    }

    pid_t cpid = fork();

    if(cpid == 0){
        int logfd;
        int result;
        char buf[100];
        char date[100];

        result = fib(n);
        logfd = open("log", O_WRONLY|O_CREAT|O_TRUNC, 0644);

        sprintf(buf, "fibonacci numbers of %i : %i\n", n, result);
        write(logfd, buf, strlen(buf));

        time_t T;
        time(&T);
        sprintf(date, "time: %s\n", ctime(&T));
        write(logfd, date, strlen(date));

        fsync(logfd);
        close(logfd);
        exit(n);

    }

    else {
        wait(&status);
        printf("My child exited with code: %i \n", n);
        exit(0);
    }







}
