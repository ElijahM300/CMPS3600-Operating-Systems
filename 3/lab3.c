/* Elijah Morris
 * Lab 3
 * 9-7-2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/file.h>
#include <signal.h>

void sigusr1_handler(int sig);

pid_t parent, cpid;
int logfd;

void sigusr1_handler(int sig) {
    if(sig == SIGUSR1) {
        write(logfd, "Signal Received", 15);
    }
}

int main() {
    int status;
    struct sigaction sa;

    sa.sa_handler = sigusr1_handler;
    sa.sa_flags = 0;
    sigfillset(&sa.sa_mask);
    sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
    sigdelset(&sa.sa_mask, SIGUSR1);
    sigaction(SIGUSR1, &sa, NULL);
    
    parent = getpid();
    cpid = fork();

    /* Child process */
    if(cpid == 0) {
        logfd = open("log", O_WRONLY|O_CREAT|O_TRUNC, 0644);
        write(logfd, "hello ", 6);
        sigsuspend(&sa.sa_mask);
        write(logfd, " world\n", 7);
        close(logfd);
        exit(0);
    }

    /* Parent process */
    else {
        sleep(2);
        kill(cpid, SIGTERM);
        kill(cpid, SIGUSR1);
        wait(&status);
        printf("Child was terminated with code %i\n", WEXITSTATUS(status));
    }

    return 0;

}


