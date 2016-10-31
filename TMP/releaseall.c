#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

int releaseall(nargs,lock_descriptor)
	int nargs;
	int lock_descriptor;
{
	int * ldesc = (int *)&lock_descriptor;
	int i = 0;
	int isError = 0;
	int j = 0;
	for(i = 0 ;i<nargs;i++)
	{
		int lock = 0;
		int lock_id = *ldesc;
		int flag = 0;
		int lock_type = 0;
		for(j = 0;j<NLOCKS;j++)
		{
			if(tab_lck[j].lck_state!=LOCK_FREE && tab_lck[j].unique_id == lock_id)
			{
				lock = j;
				flag = 1;
				lock_type = tab_lck[j].ltype;
				break;	
			}
		}
		if(flag ==1)
		{
			if(tab_lck[lock].procID[currpid]==1)
			{
				if(lock_type==READ)
				{
					rm_rlck(lock_id);
				}
				else if(lock_type==WRITE)
				{
					rm_wlck(lock_id);
				}
				//kprintf("LOCK RELEASED: %d \n",lock_id);
			}
			else
			{
				//kprintf("		TRYING TO RELEASE UNHELD LOCK: %d \n",lock_id);
				isError = 1;
			}
		}
		else
		{
			//kprintf("		UNABLE TO RELEASE LOCK: %d \n",lock_id);
			return SYSERR;
		}
		ldesc++;
	}
	if(isError==0)
	{
		 return OK;
	}
	else
	{
		 return SYSERR;
	}
}
