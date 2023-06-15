/* Elijah Morris
   xwin2.c copied from xwin89.c
   modifications:


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

struct Global {
	Display *dpy;
	Window win;
	GC gc;
	int xres, yres;
    int rect;
} g;

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
int numwin = 1;

int main(int argc, char *argv[], char *env[])
{
    ball.pos[0] = 100;
    ball.pos[1] = 0;
    ball.vel[0] = 0;
    ball.vel[1] = 0;

    my_argv = argv;
    my_env = env;
    (void)argc;

	XEvent e;
	int done = 0;
    g.rect = 0;
	x11_init_xwindows();
	while (!done) {
		/*Check the event queue*/
		while (XPending(g.dpy)) {
			XNextEvent(g.dpy, &e);
			check_mouse(&e);
			done = check_keys(&e);
			//render();
		}
        //move ball
        ball.pos[0] += ball.vel[0];
        ball.pos[1] += ball.vel[1];
        ball.vel[1] += GRAVITY;
        if(ball.pos[1] + 50 > g.yres) {
            ball.pos[1] = g.yres - 50;
            ball.vel[1] = -ball.vel[1] * 0.5;
            ball.vel[0] += 1;
        }
        render();

        usleep(100000);
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
        my_argv[0][1] = 1;
        printf("%i\n",my_argv[0][1]);
        //if(my_argv[0][1] == 1) {
          //  my_argv[0][1] = 2;
           // printf("%i\n", my_argv[0][1]);
            execve("./xwin2", my_argv, my_env);
       // }

    }
    else {
        //parent process
        if(numwin == 2) {
            printf("Number of windows cannot exceed 2!\n");
        }
        numwin++;
    }
}

int check_keys(XEvent *e)
{
	int key;
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_1:
                if(numwin < 2) {
                    make_new_window();
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
				return 1;
		}
	}
	return 0;
}

void render(void)
{
	XSetForeground(g.dpy, g.gc, 0x00000000);
	x11_clear_window();
    drawString("Elijah", g.xres / 2, g.yres / 2);
    if(g.rect){
        drawRectangle(g.xres / 2 - 25, g.yres / 2 - 25, 75, 30);
    }
    fillArc(ball.pos[0],ball.pos[1],50,50, 0*64, 360*64);
}

