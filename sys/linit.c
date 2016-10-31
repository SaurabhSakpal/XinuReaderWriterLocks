#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>
#include <q.h>

void linit(void)
{
	int m;
	struct  lock_type  *lp;
	for (m=0 ; m<NLOCKS; m++)
	{
		int j = 0;
                (lp = &tab_lck[m])->lck_state = LOCK_FREE;
		lp->unique_id = 0;
                lp->lqtail = 1 + (lp->lqhead = newqueue()+NLOCKS);
		lp->l_count = 1;
		for(j = 0;j<NPROC;j++)
		{
			lp->procID[j]= 0;
		}
		lp->num_read = 0;
        }
}
