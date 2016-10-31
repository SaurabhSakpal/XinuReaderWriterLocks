#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>
#include <sleep.h>
#include <q.h>

SYSCALL sem_post(int lock)
{
	/* STRUCTURE AND IDEA TAKEN FROM SSIGNALN() and SIGNAL() METHOD OF XINU SEMAPHORE*/
 	STATWORD ps;
        struct  lock_type  *lck_ptr;
	//kprintf("	INSIDE LSIGNAL(): %d\n",currpid);	
        disable(ps);
        if (isbadlocks(lock) || tab_lck[lock].lck_state==LOCK_FREE)
	{
                restore(ps);
                return(SYSERR);
        }
        lck_ptr = &tab_lck[lock];
	int tail = tab_lck[lock].lqtail;
	int prior_tail = q[tail].qprev;
	if(q[tail].qprev < NPROC && q[prior_tail].lock_type==WRITE)
	{
		if ((lck_ptr->l_count++) < 0)
                {
                	ready(getlast(lck_ptr->lqtail), RESCHNO);
                }
		tab_lck[lock].ltype = WRITE;
	}
	else
	{
        	while((q[tab_lck[lock].lqtail].qprev < NPROC) && (q[q[tab_lck[lock].lqtail].qprev].lock_type!=WRITE))
		{
                	if ((lck_ptr->l_count++) < 0)
			{
                        	ready(getlast(lck_ptr->lqtail), RESCHNO);
			}
			tab_lck[lock].ltype = READ;
		}
	}
	//kprintf("	OUTSIDE LSIGNAL() %d\n",currpid);
        resched();
        restore(ps);
        return(OK);        
}

SYSCALL sem_wait(int lock, int priority,int lock_type)
{
	/* IDEA AND STRUCTURE TAKEN FROM WAIT() METHOD OF XINU SEMAPHORE*/
        STATWORD ps;
        struct  lock_type  *lck_ptr;
        struct  pentry  *pptr;
	//kprintf("	INSIDE LOCK_WAIT() %d \n",currpid);
        disable(ps);
        if(isbadlocks(lock)||(lck_ptr= &tab_lck[lock])->lck_state==LOCK_FREE)
	{
                restore(ps);
                return(SYSERR);
        }

        if(--(lck_ptr->l_count)<0)
	{
		//kprintf("		VALUE OF LOCK_CNT: %d PID: %d \n",lptr->l_cnt,currpid);
               (pptr = &proctab[currpid])->pstate = PRWAIT;
                pptr->psem = lock;
                prio_enqueue(currpid,lck_ptr->lqhead,priority,lock_type,clktime);
		//kprintf("       ###### ###### ###### %d : %d : %d\n",currpid,q[q[(locks[lock].lqtail)].qprev].qkey,priority);
                pptr->pwaitret = OK;
                resched();
                restore(ps);
                return pptr->pwaitret;
        }
	//kprintf("       OUTSIDE LOCK_WAIT() %d \n",currpid);
        restore(ps);
        return(OK);
}

int get_rlck(int ldes, int priority, int lock_type)
{
	//kprintf("\n	INSIDE ACQ_READ_LOCK: ( %d , %d , %d)\n",ldes,priority,currpid);
	int success;
	int i =0;
	int lock = 0;
	int flag = 0;
	for(i = 0;i<NLOCKS;i++)
	{
		if(tab_lck[i].unique_id == ldes)
		{
			if(tab_lck[i].lck_state == LOCK_FREE)
			{	
				return DELETED;
			}
			lock = i;
			flag = 1;
			break;
		}
	}
	if(flag==0)
	{
		//kprintf("     		LOCK ALREADY DELETED: SYSERR : %d \n",currpid);
		return SYSERR;
	}
	//kprintf("       Enters Here %d : %d : %d\n",currpid,q[q[(locks[lock].lqtail)].qprev].qkey,priority);
	
	int tail = tab_lck[lock].lqtail;
	int index = q[tail].qprev;
	if(q[index].qkey > priority)
	{
		//kprintf("		Enters Here\n");
		success = sem_wait(lock,priority,lock_type);
		if(success==OK||success!=DELETED)
		{
			tab_lck[lock].ltype = READ;
			tab_lck[lock].num_read = tab_lck[lock].num_read+1;
			tab_lck[lock].procID[currpid] = 1;
			//kprintf("		1. LOCK ACQUIRED %d \n",currpid);
		}
		else
		{
			//kprintf("       	LOCK ALREADY DELETED: SYSERR : %d \n",currpid);
			return DELETED;
		}
	}
	else
	{
		if(tab_lck[lock].num_read==0)
		{
			success = sem_wait(lock,priority,lock_type);
			if(success==OK || success!=DELETED)
			{
				//kprintf("               2. FIRST READ LOCK ACQUIRED %d:%d \n",ldes,currpid);
				tab_lck[lock].ltype = READ;
				tab_lck[lock].procID[currpid] = 1;
				tab_lck[lock].num_read = tab_lck[lock].num_read +1;
				//kprintf("       OUTSIDE ACQ_READ_LOCK() %d \n",currpid);
				return success;
			}
			else
			{
				//kprintf("	       LOCK ALREADY DELETED: SYSERR : %d \n",currpid);
				return DELETED;
			}
		}
		else
		{
			//kprintf("               3. SUBSEQUENT READ LOCK ACQUIRED %d:%d \n",ldes,currpid);
			tab_lck[lock].num_read = tab_lck[lock].num_read +1;
			tab_lck[lock].ltype = READ;
			tab_lck[lock].procID[currpid] = 1;
			//kprintf("	OUTSIDE ACQ_READ_LOCK() %d \n",currpid);
			return OK;
		}	
	}	
	return success;
}

void rm_rlck(int ldesc)
{
	//kprintf("\n INSIDE REL_READ_LOCK() %d:%d \n",currpid,ldesc);
	int lock = 0 ;
        int i = 0;
	int ret = 0;
        int flag = 0;
        for(i = 0;i<NLOCKS;i++)
        {
                if(tab_lck[i].unique_id == ldesc)
                {
			if(tab_lck[i].lck_state == LOCK_FREE)
			{
				return DELETED;
			}
                        lock = i;
                        flag = 1;
                        break;
                }
        }
        if(flag==0)
        {
		//kprintf("       	LOCK ALREADY DELETED: SYSERR : %d \n",currpid);
                return SYSERR;
        }
	if(tab_lck[lock].ltype == READ&& tab_lck[lock].num_read>1)
	{
		tab_lck[lock].num_read = tab_lck[lock].num_read-1;
		tab_lck[lock].procID[currpid] = 0;
		ret = OK;
		 //kprintf("\n             1. SUBSEQUENT READ LOCK RELEASED %d:%d \n",currpid,ldesc);
	}
	else if(tab_lck[lock].ltype == READ&& tab_lck[lock].num_read==1)
	{
		tab_lck[lock].num_read = 0;
		tab_lck[lock].procID[currpid] = 0;
		ret = sem_post(lock);
		//kprintf("\n     	2. LAST READ LOCK RELEASED %d:%d \n",currpid,ldesc);
	}
	//kprintf("\n	OUTSIDE REL_READ_LOCK() %d:%d \n",currpid,ldesc);
	return ret;
}

int get_wlck(int ldesc,int priority,int lock_type)
{
	//kprintf("\n	INSIDE ACQ_WRITE_LOCK() : %d ( %d , %d , %d)\n",currpid,ldesc,priority,lock_type);
	int lock = 0 ;
	int i = 0;
	int flag = 0;
	for(i = 0;i<NLOCKS;i++)
        {
                if(tab_lck[i].unique_id == ldesc)
                {
			if(tab_lck[i].lck_state == LOCK_FREE)
			{
				return DELETED;
			}
                        lock = i;
                        flag = 1;
                        break;
                }
        }
        if(flag==0)
        {
		//kprintf("       	LOCK ALREADY DELETED: SYSERR : %d \n",currpid);
                return SYSERR;
        }
	//kprintf("       ########## Enters Here %d : %d \n",currpid,priority);
	int success = sem_wait(lock,priority,lock_type);
	if(success==OK)
	{
		tab_lck[lock].ltype = WRITE;
		tab_lck[lock].num_read = 0; 
		tab_lck[lock].procID[currpid] = 1;
		//kprintf("		WRITE LOCK ACQUIRED: %d:%d\n",currpid,ldesc);
	}
	else if(success==DELETED)
	{
		//kprintf("       	LOCK ALREADY DELETED: SYSERR : %d \n",currpid);
		return DELETED;
	}
	//kprintf("	OUTSIDE ACQ_WRITE_LOCK() %d:%d\n",currpid,ldesc);
	return success;
}

int  rm_wlck(int ldes)
{
	//kprintf("       INSIDE REL_WRITE_LOCK() %d:%d\n",currpid,ldes);
	int i =0;
        int lock = 0;
        int flag = 0;
        for(i = 0;i<NLOCKS;i++)
        {
                if(tab_lck[i].unique_id == ldes)
                {
			if(tab_lck[i].lck_state == LOCK_FREE)
			{
				return DELETED;
			}
                        lock = i;
                        flag = 1;
			break;
                }
        }
        if(flag==0)
        {
		//kprintf("       	LOCK ALREADY DELETED: SYSERR %d:%d\n",currpid,ldes);
                return SYSERR;
        }
	//kprintf("       OUTSIDE REL_WRITE_LOCK() %d:%d\n",currpid,ldes);
	tab_lck[lock].procID[currpid] = 0;
	return sem_post(lock);
}

SYSCALL lock(int ldes, int type, int priority)
{	
	if(type==READ)
	{
		//kprintf("ACQUIRE READ LOCK: %d\n",currpid);
		return get_rlck(ldes,priority,READ);
	}
	else if(type==WRITE)
	{
		//kprintf("ACQUIRE WRITE LOCK: %d\n",currpid);
		return get_wlck(ldes,priority,WRITE);
	}
}
