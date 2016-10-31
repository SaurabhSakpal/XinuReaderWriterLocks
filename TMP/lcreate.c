#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlock();

SYSCALL lcreate()
{
/*STRUCTURE AND IDEA TAKEN FROM SCREATE() METHOD USED IN XINU SEMAPHORE*/


	//kprintf("       INSIDE LCREATE() \n");
        STATWORD ps;
        int     lock_id;
        disable(ps);
        if ((lock_id=newlock())==SYSERR )
	{
                restore(ps);
                return SYSERR;
        }
        //tab_lck[lock_id].l_count = 1;
	//kprintf("	OUTSIDE LCREATE(): %d\n",lock);
        restore(ps);
        return lock_id;
}

LOCAL int newlock()
{
	int	m;
        int     lock;
        for (m=0 ; m<NLOCKS ; m++)
	{
                lock=nextlock--;
                if (nextlock < 0)
		{
                        nextlock = NLOCKS-1;
		}
                if (tab_lck[lock].lck_state==LOCK_FREE)
		{
			tab_lck[lock].ltype = 0;
			unique_lock_identifier = unique_lock_identifier +1;
			tab_lck[lock].unique_id = unique_lock_identifier;
                        tab_lck[lock].lck_state = LOCK_USED;
                        return unique_lock_identifier;
                }
        }
        return SYSERR;
}

