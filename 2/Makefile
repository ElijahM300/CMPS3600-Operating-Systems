# CMPS 3600 Week 2
# Makefile for code in examples/wk2

all: lab2 alarm fork1 fork2 fork-wait perror fork3 fork4

lab2: lab2.c
	gcc lab2.c -Wall -olab2

alarm: alarm.c
	gcc alarm.c -Wall -oalarm

fork1: fork1.c
	gcc fork1.c -Wall -ofork1

fork2:	fork2.c
	gcc fork2.c -Wall -ofork2

fork3:	fork3.c
	gcc fork3.c -Wall -ofork3

fork4:	fork4.c 
	gcc fork4.c -Wall -ofork4

# demonstrates sending signals from to parent child and use of wait
fork-wait:	fork-wait.c 
	gcc fork-wait.c -Wall -ofork-wait

# demonstrates use of perror
perror:	perror.c 
	gcc perror.c -Wall -operror

clean:  # start from scratch 
	rm -f *.o fork1 fork2 fork-wait perror fork3
	rm -f alarm out log err fork4 lab2
