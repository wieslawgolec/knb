#include "prots.h"

int get_index_by_queue(char *nick)
{
    int i, j;

    for(i = 0, j = -1; i < queues && j == -1; i++)
        if(!strcasecmp(nick, queue[i].nick)) j = i;

    return j;
}

void exchange_queue(int s1, int s2)
{
    struct knb_queue tmp;

    memset(&tmp, 0, sizeof(tmp));

    strcpy(tmp.nick, queue[s1].nick);
    tmp.used = queue[s1].used;
    tmp.prio = queue[s1].prio;
    tmp.send = queue[s1].send;

    strcpy(queue[s1].nick, queue[s2].nick);
    queue[s1].used = queue[s2].used;
    queue[s1].prio = queue[s2].prio;
    queue[s1].send = queue[s2].send;

    strcpy(queue[s2].nick, tmp.nick);
    queue[s2].used = tmp.used;
    queue[s2].prio = tmp.prio;
    queue[s2].send = tmp.send;
}

void sort_queue(int type)
{
    int i, j;

    for(i = 0; i < queues - 1; i++)
        for(j = 0; j < queues - 1 - i; j++)
            switch(type)
            {
                case 0: if(queue[j + 1].prio > queue[j].prio)
                            exchange_queue(j + 1, j);
                        break;
                case 1: if(queue[j + 1].used == true && queue[j].used == false)
                            exchange_queue(j + 1, j);
                        break;
            }
}

bool rem_queue(int index)
{
    if(!queues || index >= queues) return false;

    memset(queue[index].nick, 0, sizeof(queue[index].nick));
    queue[index].used = false;
    queue[index].send = false;
    queue[index].prio = NO_PRIO;
    if(queues > 1)
	sort_queue(1);
    queues--;
    return true;
}

bool clear_queue()
{
    if(!queues) return false;
    while(queues)
	rem_queue(0);
    return true;
}

bool add_queue(char *nick, int prio)
{
    int i = get_index_by_queue(nick);

    if(queues >= MAX_QUEUE) return false;

    if(i == -1)
    {
        strcpy(queue[queues].nick, nick);
        queue[queues].used = true;
        queue[queues].prio = prio;
	queue[queues].send = false;
        queues++;
	if(queues > 1)
    	    sort_queue(0);
        return true;
    }
    else
    {
        if(queue[i].prio != prio)
        {
            queue[i].prio = prio;
	    if(queues > 1)
        	sort_queue(0);
        }
        return true;
    }
}



void ins_queue(void)
{
    int i;

    for(i = 0; i < me.letters; i++)
        if(me.letter[i].status == 2 && me.letter[i].time && now >= me.letter[i].time)
        {
    	    if(add_queue(me.letter[i].nick, HIGH_PRIO))
	    {
    		me.letter[i].time = 0;
    		me.letter[i].status = 1;
	    }
	    else
		break;
        }
}

void build_queue(void)
{
    int i;

    for(i = 0; i < me.letters; i++)
	if(!me.letter[i].status)
	    if(!add_queue(me.letter[i].nick, LOW_PRIO))
		break;
}

void build_squeue(void)
{
    int i, j, count, count2;
    int tab[MAX_NICKS];
    int tab2[MAX_NICKS];
    
    for(i = 0, count = 0, count2 = 0; i < me.letters; i++)
	if(!me.letter[i].status)
	{
	    if(!me.letter[i].temp)
		tab[count++] = i;
	    else
		tab2[count2++] = i;
	}

    while(count) /* wpierw te z configa (wyzszy priorytet) */
    {
	j = rand() % count; /* randomowa litera z tablicy */
		
	if(!add_queue(me.letter[tab[j]].nick, LOW_PRIO))
	    break;

	for(i = j; i + 1 < count; i++)
	    tab[i] = tab[i + 1];
	
	count--;
    }    

    while(count2) /* teraz tempy (nizszy priorytet) */
    {
	j = rand() % count2; /* randomowa litera z tablicy */
		
	if(!add_queue(me.letter[tab2[j]].nick, LOW_PRIO))
	    break;

	for(i = j; i + 1 < count2; i++)
	    tab2[i] = tab2[i + 1];
	
	count2--;
    }    
}
