# CMPS 360 Week 02
# Makefile for code in /examples/week02

all:	lab3 sigmask sighand1 sighand2 perror sighand3 readwrite handler_masks readwrite1 readwrite2

lab3: lab3.c
	gcc lab3.c -Wall -olab3

# demonstrates open and fopen
readwrite: readwrite.c
	gcc readwrite.c -orw

# readwrite program with warnings removed
readwrite1: readwrite1.c
	gcc readwrite1.c -Wall -orw1

# readwrite program with strict ansi standards applied
readwrite2: readwrite2.c
	gcc readwrite2.c -Wall -ansi -pedantic -orw2

# this source demonstrates sending signals
sigmask: sigmask.c
	gcc sigmask.c -Wall -o sigmask

# this source demonstrates simple signal handling 
sighand1: sighand1.c 
	gcc sighand1.c -Wall -o sighand1

# demonstrates signal handling within a signal handler
sighand2: sighand2.c 
	gcc sighand2.c -Wall -o sighand2

# demonstrates use of perror
perror: perror.c 
	gcc perror.c -Wall -o perror

# demonstrates signals from parent - child and handler 
sighand3: sighand3.c 
	gcc sighand3.c -Wall -o sighand3

handler_masks: handler_masks.c
	gcc handler_masks.c -Wall -o handler_masks

clean:  # start from scratch 
	rm -f *.o
	rm -f sigmask sighand1 sighand2 perror sighand3 rw handler_masks rw1 rw2
