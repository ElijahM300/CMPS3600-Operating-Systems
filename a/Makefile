# Makefile for examples week-10

all: lab10 pipetest dotprod pgflt_test cache_check test rlimit

lab10: lab10.c
	gcc lab10.c -Wall -olab10 -lpthread

pipetest: pipetest.c
	gcc pipetest.c -Wall -opipetest -lpthread

dotprod: dotprod.c
	gcc dotprod.c -Wall -odotprod -lpthread

pgflt_test: pgflt_test.c
	gcc pgflt_test.c -Wall -opgflt_test

cache_check: cache_check.c
	gcc cache_check.c -Wall -ocache_check

test: test.c
	gcc test.c -Wall -otest

rlimit: rlimit.c
	gcc rlimit.c -Wall -orlimit

clean:
	rm -f *.o lab10 pipetest dotprod pgflt_test cache_check
	rm -f test rlimit a.out

