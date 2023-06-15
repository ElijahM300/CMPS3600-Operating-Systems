/* filename: timedwait.c
   purpose: CMPS 360 week 05  - example code


   Demonstrate the two POSIX thread synchronization primitives - mutex and 
   the condition variable. Demonstrate the condition using the 
   cond_timedwait() function, which you should use instead of sleep since 
   sleep applies to the entire process not just a single thread.

   Primitives used:

   int pthread_mutex_init (pthread_mutex_t *__mutex,
             __const pthread_mutexattr_t *__mutexattr)


   int pthread_cond_timedwait(pthread_cond_t *restrict cond,
              pthread_mutex_t *restrict mutex,
              const struct timespec *restrict abstime);

   Read relevant manpages for more detailed information. To compile and link
   you must link in the pthread and librt libraries:

           $ gcc timedwait.c -lpthread -lrt     */

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <stdlib.h>
#include <pthread.h>

void * p1_func( void *ptr );
void * p2_func( void *ptr );

pthread_t t1, t2;
pthread_mutex_t mutex;
pthread_cond_t cond;
struct timespec ts;

main()
{
     char *msg1 = "Hello     ";
     char *msg2 = "World    \n";

     /* initialize a mutex */
     pthread_mutex_init(&mutex, NULL);

     /* create t1, use default attributes, pass print_msg function */
     if ( (pthread_create( &t1, NULL, p1_func, (void *) msg1)) < 0) {
         perror("pthread_create");
         exit(EXIT_FAILURE);
     }

     if ( (pthread_create( &t2, NULL, p2_func, (void *) msg2) ) < 0) {
         perror("pthread_create");
         exit(EXIT_FAILURE);
     }

      /* rejoin from threads */
    if (  (pthread_join(t1, NULL)) < 0 ) {
         perror("pthread_join");
         exit(EXIT_FAILURE);
     }
     if (  (pthread_join(t2, NULL)) < 0 ) {
         perror("pthread_join");
         exit(EXIT_FAILURE);
     }


     exit(0);
}


void * p1_func( void *ptr )
{
    char *message;
    message = (char *) ptr;
    int rc; /* return code */
    pthread_mutex_lock(&mutex); 

    clock_gettime(CLOCK_REALTIME, &ts);  /* must link in librt library */
    ts.tv_sec += 3;
    rc = 0; /* return code is ETIMEDOUT when 3 seconds have passed */ 

    /* this will force a timeout */
    while ( rc == 0)
       rc = pthread_cond_timedwait(&cond, &mutex, &ts); 


    pthread_mutex_unlock(&mutex);
    write(1,message,10);
}

void * p2_func( void *ptr )
{
     char *message;
     message = (char *) ptr;
     fprintf(stderr,"%s ", message);
}
