#include "prots.h"

void got_join(char *who, char *ch)
{
	char n[NICK_LEN+1];
	int i;

	get_nick(who, n);

	if(!strcmp(me.nick, n))
        {
    	    if((i = get_index_by_chan_f(ch)) != -1)
	    {
		if(strcasecmp(me.channel[i].name_f, ch))
		    strncpy(me.channel[i].name_f, ch, CHAN_LEN);
        	me.channel[i].status = 2;
	    }
        }
}

void got_invite(char *from, char *to)
{
	char n[NICK_LEN+1];

	if(!(user = first_user) || check_flags(from))
	{
		int i, j;
		char ch[CHAN_LEN + 1];
		
		memset(ch, 0, sizeof(ch));
		
		if(to[0] == '!')
		{
		    ch[0] = '!';
		    for(i = 1, j = 0; i < strlen(to) && i < CHAN_LEN; i++)
		    {
			if(++j > 5)
			{
			    ch[strlen(ch)] = to[i];
			}
		    }
		}
		else
		    strncpy(ch, to, CHAN_LEN);
		    
		get_nick(from, n);

		i = get_index_by_chan(ch);
		
		if(i == -1)
		{
		 	if(me.channels == MAX_CHANS)
		 		send_reply(n, "-+- Cannot join `%s' (Channels limit reached)", to);
		 	else
		 	{
		 		if(strlen(ch) > CHAN_LEN)
		 			send_reply(n, "-+- Cannot join `%s' (Channel name too long)", to);
		 		else
		 		{
					add_channel(ch, "");
		 	 		if(has_penalty() < 10)
		 	 		{
		 	 			me.channel[me.channels - 1].status = 1;
		 	 			sock_write(me.sock, "JOIN %s\r\n", ch);
		 	 			add_penalty(3);
		 	                }
		 	                else
		 	                {
		 	                	me.channel[me.channels - 1].status = 0;
		 	                }
		 	         }
		 	}
		}
		else
		{
		    switch(me.channel[i].status)
		    {
			case 0:
			    if(has_penalty() < 10)
			    {
				me.channel[i].status = 1;
		 		sock_write(me.sock, "JOIN %s\r\n", me.channel[i].name);
		 		add_penalty(3);
			    } break;
			case 1:
			    send_reply(n, "-+- Allready joining `%s'", me.channel[i].name);
			    break;
			case 2:
			    send_reply(n, "-+- I'm allready on `%s'", to);
			    break;
		    }
		}
	}
}

void got_kick(char *who, char *nick, char *from)
{
	int i;
	
	if(!strcmp(me.nick, nick))
	{
		i = get_index_by_chan_f(from);

		if(i == -1) /* to niepowinno sie zdazyc */
			return;

		if(has_penalty() < 10)
		{

			me.channel[i].status = 1;
			sock_write(me.sock, "JOIN :%s %s\r\n", me.channel[i].name, me.channel[i].key);
			add_penalty(3);
	        }
	        else
	        {
	         	me.channel[i].status = 0;
	        }
        }
}

void got_quit(char *who, char *why)
{
	int i;
	char n[NICK_LEN+1];

	get_nick(who, n);

	if((i = get_index_by_letter(n)) != -1)
	{
		/* czy byl kolid, KILL line lub local kill ? */
        	if(!strncasecmp(why, "local", 5) || !strncasecmp(why, "kill", 4))
        	{
        		me.letter[i].status = 2;
        		me.letter[i].time = now + NICK_UNAVAILABLE;
			
			/* sprawdzamy czy jest cos w kolejce z tym nickiem */
			/* jezeli jest i nei bylo jeszce wyslane do serwera usuwamy */
			if((i = get_index_by_queue(n)) != -1)
			    if(!queue[i].send)
				rem_queue(i);
        	}
		else
		{
//		    /* dodajemy do kolejki z wysokim priorytetem */
//		    if(!me.catched) /* jezeli nie mamy zlapanego nicka */
//			add_queue(n, HIGH_PRIO);
		/* symulujemy zmiane nicka */
		    got_nick(n, "-quit");
		}
	}
}

void got_ison(char *data)
{
	char arg[MAX_NICKS][NICK_LEN+1];
	int i, j;

	str2words(arg[0], data, MAX_NICKS, NICK_LEN+1);
	
    	for(i = 0, j = 0; i < me.letters; i++)
    	{
		if(me.letter[i].status != 3)
		{
	    		switch(me.letter[i].status)
	    		{
				case 2:
	    	    			if(!me.letter[i].time)
					    me.letter[i].status = 0;
		    			break;
				default:
					me.letter[i].status = 0;
					break;
		    	}
	    	}
	    	if(!strcasecmp(me.letter[i].nick, arg[j]))
	    	{
			if(me.letter[i].time)
		    		me.letter[i].time = 0;
			me.letter[i].status = 1;
			j++;
	    	}
	}
	me.ison = false;
	//build_queue();
	build_squeue();
	if(queues == 1)
	{
	    me.special.check = true;
	    me.special.count = 0;    
	    me.special.max = SPECIAL_COUNT + (rand() % SPECIAL_RANDOMIZE);
	}
}

void got_nick(char *who, char *new_nick)
{
	int i, j;
        char n[NICK_LEN+1];

        get_nick(who, n);

#ifdef DEBUG
	printd("-+- %s is now know as %s (me.catched: %d)", n, new_nick, me.catched);	
#endif

        if(!strcmp(me.nick, n)) // ja zmienilem nick?
        {
        	strcpy(me.nick, new_nick);
        	add_penalty(1);
		me.send = false;
		
		/* usuwamy z kolejki jak cos jest */
		if((i = get_index_by_queue(new_nick)) != -1)
		    rem_queue(i);

                if(strcasecmp(n, new_nick) != 0) // jezeli nicki sie roznia
                {
                	if(me.catched)
                	{
                		if(isdigit((int) me.nick[0]))
                		{
                			if((j = get_index_by_letter(n)) != -1)
					{
                				me.letter[j].time = now + NICK_UNAVAILABLE;
						me.letter[j].status = 2;
					}
					me.catched = false;
                		}
				else
				{
        			    if((i = get_index_by_letter(me.nick)) != -1)
        			    {
        				me.catched = true;
        	        		/* clear queue */
					clear_queue();
				    }
				    else
				    {
					me.catched = false;
				    }
				}
                	}
                	else
                	{
        			if((i = get_index_by_letter(me.nick)) != -1)
        			{
        				me.catched = true;
        	        		/* clear queue */
					clear_queue();
        			}
        			else
        			{
        				me.catched = false;
        			}
        		}
        	}
        }
        else
        {       
		if(strcasecmp(n, new_nick) != 0) // jezeli sie roznia
        	{
        		if(!me.catched)
        		{
        			/* ktos zalapal litere */
        			if((j = get_index_by_letter(new_nick)) != -1)
        			{
        				if(me.letter[j].time) me.letter[j].time = 0;
        				me.letter[j].status = 1;
        			        /* sprawdzamy czy jest gdzies w kolejce sprawdzanei tego nicka */
					if((i = get_index_by_queue(new_nick)) != -1) /* jezeli tak, to.. */
					    if(!queue[i].send) /* jezeli jeszcze nie wyslalem tego to.. */
						rem_queue(i);
        			}
        			/* ktos starcil litere */
        			if((i = get_index_by_letter(n)) != -1)
        			{
                                	if(isdigit((int) new_nick[0]))
                                		me.letter[i].time = now + NICK_UNAVAILABLE;
                                	else
                                		me.letter[i].status = 0;
                                	/* dodawanei do kolejki z HIGH prio */
					add_queue(me.letter[i].nick, HIGH_PRIO);
                                }
                	}
                }
        }
}

void got_fail(char *nick, int reason)
{
	int j;

    	if(me.conn < 3)
    	{
	    if(!me.noaltnick.var)
	    {
        	if(me.altnick.var)
        	{
    	    		generate_nick(me.nick);
	    		if(!strlen(me.nick))
	    			rand_nick(me.nick);
	    	}
		else
		{
			strcpy(me.nick, "0");
                }
		sock_write(me.sock, "NICK %s\r\n", me.nick);
		add_penalty(1);
	    }
	    else
	    {
		sock_write(me.sock, "NICK %s\r\n", me.nick);
		add_penalty(1);
	    }
	    return;
	}

	if((j = get_index_by_queue(nick)) != -1)
	{
	    me.send = false;
	    rem_queue(j);
	}
    	if((j = get_index_by_letter(nick)) != -1)
   	{
		if(me.special.check)
		{
	    		switch(reason)
	    		{
				case 2:
					if(me.letter[j].time) me.letter[j].status = 2;
					else me.letter[j].status = 0;
					break;
				default:
					me.letter[j].status = reason;
	    		}

	    		me.special.count++;

	    		if((me.special.count == me.special.max) || (me.letter[j].status == 1 || me.letter[j].status == 3))
	    		{
				me.special.check = false;
				me.special.max = 0;
				me.special.count = 0;
	    		}
	    		else
			{
			    if(!me.letter[j].status)
				add_queue(me.letter[j].nick, MID_PRIO);
			}
		}
		else
		{
	    		switch(reason)
	    		{
				case 2:
					if(me.letter[j].time) me.letter[j].status = 2;
					else me.letter[j].status = 0;
					break;
				default:
					me.letter[j].status = reason;
	    		}
		}

   	}

}

void got_ctcp(char *from, char *what)
{
    char arg[3][MIN_LEN];
    char n[NICK_LEN+1];

    switch(me.ctcp)
    {
	case 2: break; /* od wszystkich */
	case 1: if(check_flags(from) || !(user = first_user))
		    break; /* od dopisanych */
		return;
	default: /* od nikogo */
		return;
    }

    if(has_penalty() > 10) return;

    get_nick(from, n);

    str2words(arg[0], what, 3, MIN_LEN);

    if(!strcmp(arg[0], "VERSION") || !strcmp(arg[0], "USERINFO"))
	send_notice(n, "\001%s %s\001", arg[0], me.ctcpreply);
    if(!strcmp(arg[0], "SYSINFO"))
	send_notice(n, "\001SYSINFO %s running on %s\001", KNB_SHORT, me.uname_str);

    if(!strcmp(arg[0], "TIME"))
    {
	char *t = ctime(&now);
	t[strlen(t) - 1] = '\0';
	send_notice(n, "\001TIME %s\001", t);
    }
    if(!strcmp(arg[0], "PING"))
	send_notice(n, "\001PING %s %s\001", arg[1], arg[2]);
    if(!strcmp(arg[0], "FINGER"))
	send_notice(n, "\001FINGER %s (%s@%s) Idle %ld seconds\001", me.realname, me.ident, me.host, (long int) now - me.idle);
    if(!strcmp(arg[0], "CLIENTINFO"))
	send_notice(n, "\001CLIENTINFO PING VERSION TIME FINGER USERINFO CLIENTINFO SYSINFO\001");

}

void got_privmsg(char *from, char *to, char *what)
{    
    if((check_flags(from) || !(user = first_user)) && cmd_prefix(what[0]))
    {
	int i;
	bool test;
	char *cmd, *args = NULL, *bp;
	
	for(i = 0, test = false; i < strlen(what) && test == false; i++)
	    if(what[i] == ' ')
		test = true;
	
	if(test)
	{
	    bp = what;
    	    while(*bp != ' ' && *bp)
		*bp++;
	    *bp++ = 0;
	
	    args = bp;
	}
	cmd = what + 1;
	
	if(has_penalty() < 13)
	    got_cmd(from, to, cmd, (args == NULL) ? "" : args);
    }
}
