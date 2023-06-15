/* Elijah Morris
 * Zombie Program
 * 8-30-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    pid_t child = fork();
    if(child == 0) {
        printf("child process...\n");
        while(true){
            //Check for message from parent 
        }
        sleep(10);
        exit(1);
    } else {
        printf("parent process...\n");
        wait(NULL);
        //usleep(10000000);
    }
    
    return 0;
}

