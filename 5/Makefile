# Makefile for week 5

all: lab5 sem_ctl sem_delete sem_get_info semtest

lab5: lab5.c
	gcc lab5.c -Wall -olab5 -lpthread -lrt

sem_ctl: sem_ctrl.c
	gcc sem_ctrl.c -Wall -o sem_ctl

sem_delete: sem_delete.c
	gcc sem_delete.c -Wall -o sem_delete

sem_get_info: sem_get_info.c
	gcc sem_get_info.c -Wall -o sem_get_info

semtest: semtest.c
	#gcc semtest.c -Wall -D_GNU_SOURCE -ansi -pedantic -o semtest
	gcc semtest.c -Wall -o semtest

clean:
	rm -f lab5 sem_ctrl sem_delete sem_get_info semtest

