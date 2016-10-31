/* queue.c - dequeue, enqueue */

#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * enqueue  --	insert an item at the tail of a list
 *------------------------------------------------------------------------
 */
int enqueue(int item, int tail)
/*	int	item;			- item to enqueue on a list	*/
/*	int	tail;			- index in q of list tail	*/
{
	struct	qent	*tptr;		/* points to tail entry		*/
	struct	qent	*mptr;		/* points to item entry		*/
	tptr = &q[tail];
	mptr = &q[item];
	mptr->qnext = tail;
	mptr->qprev = tptr->qprev;
	q[tptr->qprev].qnext = item;
	tptr->qprev = item;
	return(item);
}


/*------------------------------------------------------------------------
 *  dequeue  --  remove an item from the head of a list and return it
 *------------------------------------------------------------------------
 */
int dequeue(int item)
{
	struct	qent	*mptr;		/* pointer to q entry for item	*/
	mptr = &q[item];
	q[mptr->qprev].qnext = mptr->qnext;
	q[mptr->qnext].qprev = mptr->qprev;
	return(item);
}

int prio_enqueue(int proc, int head, int priority,int lock_type,unsigned long clock )
{
	/* STRUCTURE AND IDEAD COPIED FROM INSERT() FUNCTION*/


        int     next;                   /* runs through list            */
        int     prev;
	//kprintf("			INSIDE PRIO_ENQUEUE() %d:%d\n",proc,priority);
        next = q[head].qnext;
        while (q[next].qkey < priority) /* tail has maxint as key       */
                next = q[next].qnext;

	//kprintf("               ******* ****** ****** VALUE OF NEXT: %d         NEXT.NEXT: %d\n",next,q[next].qnext);	
	if(lock_type==WRITE)
	{
		//kprintf("		******* ****** ****** VALUE OF NEXT: %d		NEXT.NEXT: %d\n",next,q[next].qnext);
        	while( (q[next].lock_type!=WRITE) && ((clock-q[next].insert_time<1))&&(q[next].qnext!=-1))
        	{
//kprintf("					QNEXT: %d	CLOCK: %ul	NEXT.CLOCK: %ul\n",q[next].qnext,clock,q[next].clock);
			next = q[next].qnext;					
        	}
	}
        q[proc].qnext = next;
        q[proc].qprev = prev = q[next].qprev;
	q[proc].insert_time = clock;
	q[proc].lock_type = lock_type;
        q[proc].qkey  = priority;
        q[prev].qnext = proc;
        q[next].qprev = proc;
	//kprintf("                       OUTSIDE PRIO_ENQUEUE() %d:%d\n",proc,priority);
        return(OK);
}

