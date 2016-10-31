/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	linit();
	lcreate();
	ldelete();
	lock();
	releaseall();
	kprintf("\n\nHello World, Xinu lives\n\n");
	return 0;
}
