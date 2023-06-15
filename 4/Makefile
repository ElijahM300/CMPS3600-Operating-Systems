# Makefile for week-4

all: lab4 shared_mem shm_io sigset sigset2 strings

lab4: lab4.c
	gcc lab4.c -Wall -olab4

shared_mem: shared_mem.c
	gcc shared_mem.c -Wall -oshared_mem

shm_io: shm_io.c
	gcc shm_io.c -Wall -oshm_io

sigset: sigset.c
	gcc sigset.c -Wall -osigset

sigset2: sigset2.c
	gcc sigset2.c -Wall -osigset2

strings: strings.c
	gcc strings.c -Wall -ostrings

clean:
	rm -f lab4 shared_mem shm_io sigset sigset2 strings
