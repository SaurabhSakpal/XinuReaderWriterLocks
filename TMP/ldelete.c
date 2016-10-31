#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

SYSCALL ldelete(int ldes)
{
	
/*STRUCTURE AND IDEA TAKEN FROM SDELETE() METHOD USED IN XINU SEMAPHORE*/

	//kprintf("	INSIDE LDELETE() %d:%d\n",currpid,ldes);
        STATWORD ps;
	int m = 0;
        int     pid;
        struct  lock_type  *lp;
        disable(ps);
	int j = 0;
	int lock_id = 0;
	int flag = 0;
	for(j=0;j<NLOCKS;j++)
	{
		if(tab_lck[j].lck_state!=LOCK_FREE&&tab_lck[j].unique_id==ldes)
		{
			lock_id = j;
			flag = 1;
			break;
		}
	}
	if(flag==0)
	{
		//kprintf("       	LOCK ALREADY DELETED:(SYSERR) \n");
		return SYSERR;
	}
        if (isbadlocks(lock_id) || tab_lck[lock_id].lck_state==LOCK_FREE) {
                restore(ps);
                return(SYSERR);
        }
        lp = &tab_lck[lock_id];
	lp->ltype = 0;
	lp->l_count=1;
	for(m = 0;m<NPROC;m++)
	{
		lp->procID[m] = 0;
	}
	lp->lck_state = LOCK_FREE;
	lp->num_read = 0;
        if (nonempty(lp->lqhead))
	{
                while( (pid=getfirst(lp->lqhead)) != EMPTY)
               	{
                	proctab[pid].pwaitret = DELETED;
                	ready(pid,RESCHNO);
                }
                resched();
        }
	//kprintf("       OUTSIDE LDELETE(): %d:%d\n",currpid,ldes);
        restore(ps);
        return(OK);
}

