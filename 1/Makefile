all: x10 xwin2 xwin89 dict

x10: x10.c
	gcc x10.c -Wall -ox10

dict: dict.c
	gcc dict.c -Wall -pedantic -ansi -o dict

xwin89: xwin89.c
	gcc xwin89.c -Wall -Wextra -Werror -pedantic -lX11 -oxwin89

xwin2: xwin2.c
	gcc xwin2.c -Wall -Wextra -Werror -pedantic -lX11 -oxwin2 -g

clean:
	rm -f a.out x10 dict xwin89 xwin2

