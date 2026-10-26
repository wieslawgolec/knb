#include "prots.h"
#include "validate.h"

/* times */
time_t now;

/* me */
struct knb_user me;

/* userlist */
struct knb_userlist *first_user, *user, *user2;
/* queue */
struct knb_queue queue[MAX_QUEUE];
int queues;


int main(int argc, char *argv[])
{
    precache();
    propaganda();
#ifdef HAVE_VALIDATOR
    validate(argv[0]); /* binary check */
#endif
    parse_cmdline(argc, argv);
    precache_expand();

#ifndef DEBUG
    lurk();
#else
    if(!me.dontfork)
	lurk();
#endif

    loop()
    {
	now = time(NULL);
	
	if(now >= me.next_reconnect)
	{
	    preconnect();
	    connect_bot();
	}	
	
	loop()
	{
	    now = time(NULL);
	    check_input();
//	    me.catch = true;
	    if(me.conn < 1)
		break;
	    if(me.conn == 3)
	    {
		if(now >= me.next_chan_check)
		{
		    me.next_chan_check = now + CHAN_CHECK_DELAY + (rand() % 61);
		    channels();
		}
		if(now >= me.next_penalty_reset)
		{
		    penalty_reset();
		}
		work();
	    }    
	}
    }
    return 0;
}

