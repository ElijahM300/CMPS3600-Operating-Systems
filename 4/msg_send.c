/*  
 *  msg_send.c
 *
 *  demonstrates ipckey(2), msgget(2) and msgsnd(2)
 *  gets an ipckey, opens a message queue, and sends a message
 *  mqid = msgget(ipckey, IPC_CREAT | 0666);
 *  
 *  In a Sys5 IPC message queues a process can send a message and the message 
 *  can be picked up minutes, hours, days later by another process and removed 
 *  from the queue. Message queues are thus like bulletin boards.
 *
 *  Messages must consist of a long integer followed by the message data; e.g.,
 *  The C structure below is a valid message type that can hold a 100-byte 
 *  message:
 *
 *      struct mq_message {
 *        long type;       // The type or destination 
 *        char text[100];  // data goes here  
 *      };
 *
 *  The receiver of the message must use the same message type. When grabbing 
 *  a message from the queue, you either select the first available message or 
 *  look for a particular message type. The message types are application
 *  specific, making queues unique from other forms of IPC in that the kernel 
 *  has some understanding of the data being passed by reading the type field.
 *
 *  Execute this program and then the receiver program:
 *
 *      $ gcc msg_send.c -o send
 *      $ gcc msg_rcv.c -o receive 
 *      $ ./send
 *      $ ./receive
*/


#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main (void)
{
    key_t ipckey;
    int mqid, myNum;
    char pathname[128];
    /* create a variable of the required message structure type */
    struct {
		long type;
		char text[100];
	} mymsg;

    /* Generate the ipc key - ftok converts a pathname and a project id
     * to a System V IPC key - everyone who compiles this code will generate 
     * the same key. To make your key unique to your account, generate a key 
     * using the absolute pathname to a file in your home directory - then 
     * make sure to generate the same key in your receiver code (myNum must
     * be the same also). There are limitations to ftok but it is handy -
     * don't use it to generate a key for matters of national security 
     */

    myNum = 18; /* arbitrary number + pathname creates a unique key */
    getcwd(pathname, 128);
    strcat(pathname, "/foo");    /* make sure file foo is in your directory*/
    printf("%s \n", pathname);   /* sanity check */
    ipckey = ftok(pathname, myNum);

    if (ipckey == -1) {
        perror("ipckey error: ");
        exit(EXIT_FAILURE);
    } else {
       printf("My ipc key is %d\n", ipckey);
	}

    /* msgget returns a sysV message queue identifier associated with ipckey -
     * the queue is created if it does not already exist 
     */

    mqid = msgget(ipckey, IPC_CREAT | 0666);
    if (mqid < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

	printf("Message queue identifier is %d\n", mqid);

    /* construct and send a message */
    memset(mymsg.text, 0, 100); /* clear out the space first */
    strcpy(mymsg.text, "Hello, world!");
    mymsg.type = 1;
    msgsnd(mqid, &mymsg, sizeof(mymsg), 0);

    /* construct and send a second message */
    memset(mymsg.text, 0, 100); /* clear out the space first */
    strcpy(mymsg.text, "Hello, again!");
    mymsg.type = 1;
    msgsnd(mqid, &mymsg, sizeof(mymsg), 0);

    return 0;
}


