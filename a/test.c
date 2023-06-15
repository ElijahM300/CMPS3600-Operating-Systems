/* test.c
 * show memory footprint of very small program 
 */

int fib(int n)
{
	if (n <= 2)
		return 1;
	return fib(n-1) + fib(n-2);
}

int main(void)
{
	fib(2);
	return 0;
}

/* 
   size a.out
   text	   data	    bss	    dec	    hex	filename
   1177	    560	      8	   1745	    6d1	a.out
*/
