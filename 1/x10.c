//Elijah Morris
//10-27-21

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[], char *envp[]) {
    int status;
    FILE *fpo = fopen("myprog.c", "w");
    fprintf(fpo, "#include <stdio.h>\n");
    fprintf(fpo, "int main(void) {\n");
    fprintf(fpo, "\tprintf(\"Hello World\\n\");\n");
    fprintf(fpo, "\treturn 0;\n");
    fprintf(fpo, "}\n");
    fclose(fpo);
    pid_t pid = fork();
    if(pid == 0) {
        //child will compile the program
        char *myarg[] = { "gcc", "myprog.c", "-Wall", "-omyprog", NULL };
        printf("child building program...\n");
        execve("/usr/bin/gcc", myarg, envp);
    }
    else {
        //sleep(2);
        wait(&status);
        //parent
        pid_t pid2 = fork();
        if(pid2 == 0) {
            //child
            char *myarg2[] = { "./myprog", NULL };
            printf("child exec program...\n");
            execve("./myprog", myarg2, envp);
        }
        else {
            wait(&status);
            //delete the executable
            unlink("myprog");
            exit(0);
        }
    }

}
