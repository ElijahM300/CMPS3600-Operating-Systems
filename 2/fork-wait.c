/*  fork-wait.c     
 *  cmps 360 week 1 examples
 *  
 * demonstrate fork(2),wait(2),kill(2) by sending signals from parent to child
 *
 *  In linux (for efficiency) the fork is cloned as a copy-on-write; i.e., 
 *  until a modification is done by either parent or child, both share the 
 *  same memory addresses for data. After the copy the virtual addresses may
 *  be identical but the actual addresses will not be.
 *
 *  Open top in one terminal window:
 *       $ top -u [username]               # hit 'q' to end top
 *
 *  Open a second terminal window. Compile and execute with 4 secs pause. 
 *
 *       $ gcc -o fork-wait fork-wait.c
 *       $ ./fork-wait 4
 *
 *  Watch the process status column. From top man page:
 *
 *  S  --  Process Status
 *         The status of the task which can be one of:
 *             D = uninterruptible sleep
 *             R = running
 *             S = sleeping
 *             T = traced or stopped
 *             Z = zombie
 *
 * Tasks shown as running should be more properly thought of as aready to run 
 * -- their task_struct is simply represented on the Linux run-queue. Even 
 * without a true SMP machine, you may see numerous tasks in this state 
 * depending on top's delay interval and nice value.  
 */
 
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>      /* header file for perror */

int status=0;  /* status in parent and status in child share same virtual 
                * address but physical address is in different user spaces - 
                * the kernel passes status from child to parent via wait call
                */
void printstat();

int main(int argc, char *argv[], char *envp[]) {
   char buf[50];
   int ret, nsecs; 
   pid_t cpid;

   /* check arg count if nsecs read from the command line */
   if (argc > 1) /* the executable filename is the first argument */
      nsecs = atoi(argv[1]);
   else 
      nsecs = 2;
 
   /* print the virtual address of status before the fork.
    * note that the virtual address of status is the same in
    * the parent and in the child - but the physical address differs! 
    * the point is that the parent and child split into two separate images
    */
   printf("Virtual address of status before fork: %p\n",&status);

   /* FORK */
   cpid = fork();

   if (cpid == 0) {     /* CHILD */

      /* modify the value of status in the child's user space */
      status=7;
      printf("Virtual address of status in child: %p\n",&status);
      printf("Value of status in child's user space: %d\n",status);
      fflush(stdout); /* if you mix printf and write you need to flush stdout */

      while (1)  {   /* put the child in an infinite sleep loop */
        sleep(1);    /* parent will wake up the child */                 
      } 

      /* the child never makes it here because parent kills it -
       * the exit code that is copied to status is the signal number 
       * that terminated the child - 
       */
      _exit(0);   /* _exit is async-safe */
   }

   if (cpid > 0) {   /* PARENT */

      printf("My child's pid is: %d\n",cpid);
      status = 99;  
      sleep(nsecs);  /* make sure the child process has time to finish printf */
      printf("Virtual address of status in parent: %p\n",&status);
      printf("Value of status in parent's virtual user space: %d\n",status);
      fflush(stdout);

      strcpy(buf,"\nSending SIGSTOP to my child. \n");
      write(1,buf,strlen(buf));
      ret = kill(cpid, SIGSTOP);  /* stop child process but do not terminate */
      if (ret < 0) { perror("kill SIGSTOP "); }
      ret = waitpid(cpid, &status, WUNTRACED|WCONTINUED);  /* non-blocking */
      printstat(ret);
 
      sleep(nsecs);
      strcpy(buf,"Sending SIGCONT to my child. \n");
      write(1,buf,strlen(buf));
      ret = kill(cpid, SIGCONT); 
      if (ret < 0) { perror("kill SIGCONT: "); }
      ret = waitpid(cpid, &status, WUNTRACED|WCONTINUED);  /* non-blocking */
      printstat(ret);

      sleep(nsecs);
      strcpy(buf,"Sending SIGHUP to my child.\n");
      write(1,buf,strlen(buf));
      ret = kill(cpid, SIGHUP);   /* sighup terminates */
      if (ret < 0) perror("kill:SIGHUP");
      ret = wait(&status);   /* blocking wait to reap exit code */
      if (ret < 0) perror("wait: ");
      printstat(ret);

      printf("My child's exit code: %d\n",status); 
      exit (EXIT_SUCCESS);
   }
}

void printstat(int pid){
  if (WIFEXITED(status)) {
     printf("Parent: child %d exited, status=%d\n", pid, WEXITSTATUS(status));
  } else if (WIFSIGNALED(status)) {
     printf("Parent: child %d killed by signal %d\n", pid, WTERMSIG(status));
  } else if (WIFSTOPPED(status)) {
      printf("Parent: child %d stopped by signal %d\n", pid, WSTOPSIG(status));
  } else if (WIFCONTINUED(status)) {
      printf("Parent: child %d continued\n",pid);
  }
  fflush(stdout);
}
