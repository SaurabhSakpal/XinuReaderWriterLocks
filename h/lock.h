#define NLOCKS		 50
#define LOCK_FREE   '\08'   
#define DELETED		3
#define LOCK_USED   '\09'
#define WRITE           2
#define READ		1
struct  lock_type
{
        char    lck_state;
        int     l_count;
        int     lqhead;
        int     lqtail;
	int	unique_id;
	int	num_read;
	int	ltype;
	int	procID[NPROC];
	
}lock_type;
extern  int     nextlock;
extern  struct  lock_type  tab_lck[];
extern	int	unique_lock_identifier;

#define isbadlocks(l)     (l<0 || l>=NLOCKS)

