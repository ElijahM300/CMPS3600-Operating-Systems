/* Elijah Morris
   xwinp3.c copied from xwinp2.c
   modifications: added a semaphore to control threads


 cs3600 Winter 2021
 program author:  Gordon Griesel
           date:  Jan 3, 2021
        purpose:  C program for students to practice their C programming
		          over the Winter break. Also, introduce students to the
				  X Window System. We will use the X Window protocol or
				  API to generate output in some of our lab abd homework
				  assignments.

 Instructions:

      1. If you make changes to this file, put your name at the top of
	     the file. Use one C style multi-line comment to hold your full
		 name. Do not remove the original author's name from this or 
		 other source files please.

      2. Build and run this program by using the provided Makefile.

	     At the command-line enter make.
		 Run the program by entering ./a.out
		 Quit the program by pressing Esc.

         The compile line will look like this:
            gcc xwin89.c -Wall -Wextra -Werror -pedantic -ansi -lX11

		 To run this program on the Odin server, you will have to log in
		 using the -YC option. Example: ssh myname@odin.cs.csub.edu -YC

      3. See the assignment page associated with this program for more
	     instructions.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/sem.h>

struct SharedMem {
    int flag;
    char myname[60];
    unsigned int parent_screen_color;
    unsigned int child_screen_color;
};

struct Global {
	Display *dpy;
	Window win;
	GC gc;
	int xres, yres;
    int rect;
    unsigned int screen_color;
    unsigned char child;
    int shmid;
    struct SharedMem *shared;
    double text1_pos_x;
    double text2_pos_x;
    double direction;
    int move_text;
    int thread1;
    int thread2;
} g;

union {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} my_semun;

const int GRAVITY = 1;

struct Ball {
    int pos[2];
    int vel[2];
} ball;

void x11_cleanup_xwindows(void);
void x11_init_xwindows(void);
void x11_clear_window(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void render(void);

char **my_argv;
char **my_env;
int status;
int semid;
int sem_value;
struct sembuf grab[2], release[1];

void *moveText(void *ptr) {
    /*Get thread number*/
    printf("thread started\n");
    int *move;
    move = (int *)ptr; 
    printf("thread %i\n", *move);

    /*Grab semaphore*/
    semop(semid, grab, 2);
    printf("thread %i got sem\n", *move);
    sem_value = 1;
    while(1) {
        /*If the sem is available, tell a thread to grab it*/
        if(sem_value == 0) {
            semop(semid, grab, 2);
            printf("thread %i got sem\n", *move);
            sem_value = 1;
        }
       
        /*If the t key has been pressed, the thread has the sem, and it's thread 0, move text*/
        if((g.move_text == 1) && *move == 0) {
            g.text1_pos_x += (5 * g.direction);      
            if(g.text1_pos_x > 140) {
                g.direction = -g.direction;
            }
            if(g.text1_pos_x < 10) {
                g.text1_pos_x = 10;
                g.direction = -g.direction;
                semop(semid, release, 1);
                sem_value = 0;
            }
        }
        /*If the t key has been pressed, the thread has the sem, and it's thread 1, move text*/
        else if((g.move_text == 1) && *move == 1) {
            g.text2_pos_x += (5 * g.direction);       
            if(g.text2_pos_x > 140) {
                g.direction = -g.direction;
            }
            if(g.text2_pos_x < 10) {
                g.text2_pos_x = 10;
                g.direction = -g.direction;  
                semop(semid, release, 1);
                sem_value = 0;
            }
        }
    }
    printf("thread exiting\n");
    pthread_exit(0);
}

int main(int argc, char *argv[], char *env[])
{
    x11_init_xwindows();
    /*
    ball.pos[0] = 100;
    ball.pos[1] = 0;
    ball.vel[0] = 0;
    ball.vel[1] = 0;
    */
    my_argv = argv;
    my_env = env;
    (void)argc;

    /*Initialize variables to be used*/
	XEvent e;
	int done = 0;
    pthread_t text1, text2; 
    int move1 = 0; 
    int move2 = 1;
    int nsems = 1;
    g.rect = 0;
    g.screen_color = 0x00ff3333;
    g.text1_pos_x = 10;
    g.text2_pos_x = 10;
    g.direction = 0.0000001;
    g.child = 0;
    g.shared = NULL;
    g.move_text = 0;
    g.thread1 = 1;
    g.thread2 = 0;
    if(argc > 1) {
        if(strcmp(argv[1], "_CHILD_") == 0) {
            g.child = 1;
        }
    }
    if(!g.child) {

        /*Set the ipckey and setup shared memory*/
        key_t ipckey;
        char pathname[128];
    
        getcwd(pathname, 128);
        strcat(pathname, "/foo");

        ipckey = ftok(pathname, 21);

        /*g.shmid = shmget(ipckey, sizeof(int), IPC_CREAT | 0666);
         * The above line has a bug in it with sizeof(int)
         * The program did not crash when run since 4 bytes of shared memory was
         * enough to handle the functions of changing the window color.
         * The bug should be fixed however, since it's good practice to always make 
         * sure there's enough memory allocated to shared memory and 4 bytes may 
         * not be enough as the program grows in complexity.
         */
        g.shmid = shmget(ipckey, sizeof(struct SharedMem), IPC_CREAT | 0666);
        g.shared = (struct SharedMem *)shmat(g.shmid, (void *) 0, 0);
        g.shared->parent_screen_color = g.screen_color;

        semid = semget(ipckey, nsems, 0666 | IPC_CREAT);
        if(semid < 0) {
            perror("semget: ");
            _exit(1);
        }
        /*Semaphore initialization*/
        my_semun.val = 0;
        semctl(semid, 0, SETVAL, my_semun);
        sem_value = semctl(semid, 0, GETVAL);
        grab[0].sem_num = 0;
        grab[0].sem_flg = SEM_UNDO; 
        grab[0].sem_op = 0;
        grab[1].sem_num = 0;
        grab[1].sem_flg = SEM_UNDO;
        grab[1].sem_op = +1;

        release[0].sem_num = 0;
        release[0].sem_flg = SEM_UNDO;
        release[0].sem_op = -1;

        /*Create threads*/
        if(pthread_create(&text1, NULL, moveText, (void *)&move1) < 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        if(pthread_create(&text2, NULL, moveText, (void *)&move2) < 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    if(g.child) {
        g.shmid = atoi(argv[2]);
        g.shared = (struct SharedMem *)shmat(g.shmid, (void *) 0, 0);
    }

	while (!done) {
		/*Check the event queue*/
		while (XPending(g.dpy)) {
			XNextEvent(g.dpy, &e);
			check_mouse(&e);
			done = check_keys(&e);
			render();
		}
        render();
        usleep(30000);
        if(!g.child) {
            if(g.shared->parent_screen_color != g.screen_color) {
                printf("parent got message\n");
                g.screen_color = g.shared->parent_screen_color;
            }
        }

        /*move ball
        ball.pos[0] += ball.vel[0];
        ball.pos[1] += ball.vel[1];
        ball.vel[1] += GRAVITY;
        if(ball.pos[1] + 50 > g.yres) {
            ball.pos[1] = g.yres - 50;
            ball.vel[1] = -ball.vel[1] * 0.5;
            ball.vel[0] += 1;
        }
        usleep(20000); 
        */
    }
    if(g.shared != NULL) {
        shmdt(g.shared);
        shmctl(g.shmid, IPC_RMID, 0);
    }
	x11_cleanup_xwindows();
	return 0;
}

void x11_cleanup_xwindows(void)
{
	XDestroyWindow(g.dpy, g.win);
	XCloseDisplay(g.dpy);
}

void x11_init_xwindows(void)
{
	int scr;

	if (!(g.dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display!\n");
		exit(EXIT_FAILURE);
	}
	scr = DefaultScreen(g.dpy);
	g.xres = 400;
	g.yres = 200;
	g.win = XCreateSimpleWindow(g.dpy, RootWindow(g.dpy, scr), 1, 1,
							g.xres, g.yres, 0, 0x00ffffff, 0x00ff4444);
	XStoreName(g.dpy, g.win, "cs3600 xwin sample");
	g.gc = XCreateGC(g.dpy, g.win, 0, NULL);
	XMapWindow(g.dpy, g.win);
	XSelectInput(g.dpy, g.win, ExposureMask | StructureNotifyMask |
								PointerMotionMask | ButtonPressMask |
								ButtonReleaseMask | KeyPressMask);
}

void x11_clear_window(void)
{
	XClearWindow(g.dpy, g.win);
}

void clear_window(void)
{
	XClearWindow(g.dpy, g.win);
}

void drawString(const char *str, const int x, const int y)
{
    XDrawString(g.dpy, g.win, g.gc, x, y, str, strlen(str));
}

void drawArc(int x, int y, int w, int h, int st, int end) {
    XDrawArc(g.dpy, g.win, g.gc, x, y, w, h, st, end);
}

void fillArc(int x, int y, int w, int h, int st, int end) {
    XFillArc(g.dpy, g.win, g.gc, x, y, w, h, st, end);
}

void drawRectangle(int x, int y, int w, int h)
{
    XDrawRectangle(g.dpy, g.win, g.gc, x, y, w, h);
}

void fillRect(int x, int y, int w, int h) {
    XFillRectangle(g.dpy, g.win, g.gc, x, y, w, h);
}

void check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;
	int mx = e->xbutton.x;
	int my = e->xbutton.y;

	if (e->type != ButtonPress && e->type != ButtonRelease)
		return;
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) { }
		if (e->xbutton.button==3) { }
	}
	if (e->type == MotionNotify) {
		if (savex != mx || savey != my) {
			/*mouse moved*/
			savex = mx;
			savey = my;
		}
	}
}

void make_new_window() {
    int cpid = fork();
    if(cpid == 0) {
        //child process 
        char shm[32];
        sprintf(shm, "%i", g.shmid);
        char *newargs[] = {"xwinp3","_CHILD_",shm, NULL};
        execve("./xwinp3", newargs, my_env);
    }
    else {
        //parent process
    }
}


int check_keys(XEvent *e) {
    static int numwin = 0;
    static int isdiffcolor = 0;
	int key;
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
            case XK_t:
                if(g.move_text == 0) {
                    g.move_text = 1;
                }
                else {
                    g.move_text = 0;
                }
                break;
			case XK_1:
                numwin++;
                if(numwin < 2 && g.child == 0) {
                    make_new_window();
                }
				break;
            case XK_g:
                if(g.child) {
                    if(isdiffcolor == 1) {
                        g.shared->parent_screen_color = g.shared->child_screen_color;
                        isdiffcolor = 0;
                    }
                    else if(isdiffcolor == 0) {
                        g.shared->parent_screen_color = 0x0000ff00;
                        g.shared->child_screen_color = g.screen_color;
                        isdiffcolor = 1;
                    }
                }
                break;
               
            case XK_u:
                if(g.rect == 0){
                    g.rect = 1;
                }
                else
                    g.rect = 0;
                break;
			case XK_Escape:
                semctl(semid, 0, IPC_RMID);
				return 1;
		}
	}
	return 0;
}

void render(void)
{
	XSetForeground(g.dpy, g.gc, g.screen_color);
    fillRect(0, 0, g.xres, g.yres);
    XSetForeground(g.dpy, g.gc, 0x00000000);
    /*fillArc(ball.pos[0],ball.pos[1],50,50, 0*64, 360*64);*/
    if(g.child) {
        drawString("I am the child.", 10, g.yres/2);
    }
    else {
        drawString("I am the parent.", 10, g.yres/2);
    }
    if(!g.child) {
        drawString("Thread #1", g.text1_pos_x, 20);
        drawString("Thread #2", g.text2_pos_x, 35);
        drawString("Press 'T' to move the text.", 10, 50);
    }
}

