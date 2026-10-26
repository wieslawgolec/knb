#include "prots.h"

void got_cmd(char *who, char *to, char *cmd, char *args)
{
    char buf[MASK_LEN];
    char arg[3][MASK_LEN];
    char n[NICK_LEN+1], u[IDENT_LEN+1], h[MASK_LEN+1];
    char target[CHAN_LEN + 1];
    
    memset(target, 0, sizeof(target));
    memset(arg, 0, sizeof(arg));
    
    str2words(arg[0], args, 3, MASK_LEN);    

    get_nuh(who, n, u, h);
    
    if(me.public_reply)
    {
	if(!strcmp(me.nick, to))
	    strncpy(target, n, NICK_LEN);
	else
	    strncpy(target, to, CHAN_LEN);
    }
    else
	strncpy(target, n, CHAN_LEN);
#ifdef DEBUG
    printd("Got cmd: from '%s!%s@%s' cmd '%s' with args '%s'", n, u, h, cmd, args);
    
    if(!strcasecmp(cmd, "debug"))
    {
	int i;
	send_reply(target, "-+- me.catch: %s, me.catched: %s, queues: %d",
	 me.catch ? "true" : "false", me.catched ? "true" : "false", queues);	 
	
	for(i = 0; i < queues; i++)
	    send_reply(target, "-+- queue[%d]: nick: %s, prio: %d", i, queue[i].nick, queue[i].prio); 
    }
#endif    

    if(!strcasecmp(cmd, "new") || !strcasecmp(cmd, "+host") || !strcasecmp(cmd, "+mask") || !strcasecmp(cmd, "+user"))
    {	
	char tn[MASK_LEN+1], tu[MASK_LEN+1], th[MASK_LEN+1];
	
	memset(tn, 0, sizeof(tn));
	memset(tu, 0, sizeof(tu));
	memset(th, 0, sizeof(th));
	
	if(!strlen(arg[0]))
	{
	    if(check_ident_prefix(u[0])) /* zamieniamy prefix identa na '?' */
		u[0] = '?';
		
	    sprintf(buf, "*!%s@%s", u, h);
	    
	    if(strlen(buf) > MASK_LEN)
	    {
		send_reply(target, "-+- Mask too long (%d chars)", strlen(buf));
		return;
	    }
	    if(check_flags(buf))
	    {
		send_reply(target, "-+- Host '%s' allready exists", buf);
		return;
	    }
	    get_nuh(buf, tn, tu, th); // maska do dodania
	}
	else
	{
	    if(strlen(arg[0]) > MASK_LEN)
	    {
		send_reply(target, "-+- Mask too long (%d chars)", strlen(buf));
		return;
	    	    
	    }
	    else if(!extendhost(arg[0], MASK_LEN, buf))
	    {
		send_reply(target, "-+- Incorrect host '%s'", arg[0]);
		return;
	    }
	    else
	    {
		if(check_flags(buf))
		{
		    send_reply(target, "-+- Host '%s' allready exists", buf);
		    return;
		}
		get_nuh(buf, tn, tu, th);   	
		
		if(strlen(tn) > NICK_LEN)
		{
		    send_reply(target, "-+- Nick too long in host `%s' (%d chars)", buf, strlen(tn));
		    return;
		}
		if(strlen(tu) > IDENT_LEN)
		{
		    send_reply(target, "-+- Ident too long in host `%s' (%d chars)", buf, strlen(tu));
		    return;
		}
	    }
	}

	user = first_user;
		
	while(user && user->next)
	    user = user->next;
    
	user2 = malloc(sizeof(struct knb_userlist));
	
	if(first_user)
	    user->next = user2;
	else
	    first_user = user2;
	    
	strncpy(user2->nick, tn, NICK_LEN);
	strncpy(user2->ident, tu, IDENT_LEN);
	strncpy(user2->host, th, MASK_LEN);	    	
    
	user2->next = NULL;
	
	send_reply(target, "-+- Host '%s!%s@%s' added", tn, tu, th);
	save_uf(me.userfile);
	return;
    }
    if(!strcasecmp(cmd, "-host") || !strcasecmp(cmd, "-mask") || !strcasecmp(cmd, "-user"))
    {
	int i = -1;
	char tn[MASK_LEN+1], tu[MASK_LEN+1], th[MASK_LEN+1];
	
	memset(tn, 0, sizeof(tn));
	memset(tu, 0, sizeof(tu));
	memset(th, 0, sizeof(th));

	if(!(user = first_user))
	{
	    send_reply(target, "-+- Userlist is empty");
	    return;
	}
	
	if(!strlen(arg[0]))
	{
	    if(check_ident_prefix(u[0])) /* zamieniamy prefix identa na '?' */
		u[0] = '?';

	    sprintf(buf, "*!%s@%s", u, h);
	    if(!check_flags(buf))
	    {
		send_reply(target, "-+- Host '%s' not found", buf);
		return; 
	    }
	    get_nuh(buf, tn, tu, th);
	}
	else
	{
	    if(!_isnumber(arg[0])) i = -1;
	    else i = (atoi(arg[0]) < 1) ? 1 : atoi(arg[0]);
	    
	    if(i == -1)
	    {
		if(strlen(arg[0]) > MASK_LEN)
		{
		    send_reply(target, "-+- Mask too long (%d chars)", strlen(arg[0]));
		    return;
		}
		else if(!extendhost(who, MASK_LEN, buf))
		{
		    send_reply(target, "-+- Incorrect host '%s'", arg[0]);
		    return;
		}
		else
		{
		    if(!check_flags(buf))
		    {
			send_reply(target, "-+- Host '%s' not found", buf);
			return;
		    }
		    get_nuh(buf, tn, tu, th);  
		    
		    if(strlen(tn) > NICK_LEN)
		    {
			send_reply(target, "-+- Nick too long in host `%s' (%d chars)", buf, strlen(tn));
			return;
		    }
		    if(strlen(tu) > IDENT_LEN)
		    {
			send_reply(target, "-+- Ident too long in host `%s' (%d chars)", buf, strlen(tu));
			return;
		    }
		}
	    }
	}
	
	if(i == -1)
	{
	    for(user = first_user; user; user2 = user, user = user->next)
		if(!strcasecmp(user->nick, tn) && !strcasecmp(user->ident, tu) && !strcasecmp(user->host, th))
		{	
		    if(user == first_user)
			first_user = first_user->next;
	    	    else
			user2->next = user->next;
		    free(user);
		
		    send_reply(target, "-+- Host '%s' removed", buf);
		    return;	
		}	

	}
	else
	{
	    int j = 1;
	    
	    for(user = first_user, j = 1; user; user2 = user, user = user->next, j++)
		if(j == i)
		{	
		    if(user == first_user)
			first_user = first_user->next;
	    	    else
			user2->next = user->next;
		    free(user);
		
		    send_reply(target, "-+- Host '%s' removed", buf);
		    return;	
		}	
	    
	    send_reply(target, "-+- Host with number '%d' not found", i);	
	    return;	
	}		
    }	   	

    if(!strcasecmp(cmd, "hosts") || !strcasecmp(cmd, "masks") || !strcasecmp(cmd, "users"))
    {
	int i;
	
	if(!(user = first_user))
	    send_reply(target, "-+- Userlist is empty");
	else
	    for(user = first_user, i = 1; user; user = user->next, i++)
		send_reply(target, "-+- [%2d] %s!%s@%s", i, user->nick, user->ident, user->host);
	return;
    }
    if(!strcasecmp(cmd, "j") || !strcasecmp(cmd, "join"))
    {
	if(!strlen(args))
	    send_reply(target, "-+- Usage: !%s <channel> [key]", cmd);
	else if(!chan_prefix(arg[0][0]))
	    send_reply(target, "-+- Incorrect channel name (Bad prefix (%s))", arg[0]);
	else if(me.channels == MAX_CHANS)
	    send_reply(target, "-+- Cannot join channel (Channel limit reached)");
	else if(strlen(arg[0]) > CHAN_LEN)
	    send_reply(target, "-+- Channel name too long (%d chars)", strlen(arg[0]));
	else if(strlen(arg[0]) > CHAN_LEN)
	    send_reply(target, "-+- Key too long for '%s' (%d chars)", arg[0], strlen(arg[1]));
	else if(get_index_by_chan(arg[0]) != -1)
	    send_reply(target, "-+- Channel allready exists on my list");
	else
	{
	    add_channel(arg[0], strlen(arg[1]) ? arg[1] : "");
	    if(strlen(arg[1]))
		sprintf(buf, "-+- Channel '%s' added with key '%s'", arg[0], arg[1]);
	    else	
		sprintf(buf, "-+- Channel '%s' added without key", arg[0]);
		
	    send_reply(target, "%s", buf);
	    sock_write(me.sock, "JOIN %s %s\r\n", arg[0], arg[1]);
	    add_penalty(3);
	}
	return;
    }
    if(!strcasecmp(cmd, "p") || !strcasecmp(cmd, "part") || !strcasecmp(cmd, "l") || !strcasecmp(cmd, "leave") || !strcasecmp(cmd, "cycle") || !strcasecmp(cmd, "rejoin"))
    {
        int i = 0;
	int j = 0;
	
	char *cp = NULL;

        if(strlen(args))
        	cp = args;
        else
        {
        	if(strcmp(me.nick, to) != 0)
		{
			cp = to;
			j = 1;
		}
        }

	if(cp == NULL)
		send_reply(target, "-+- Usage: !%s <channel>", cmd);
        else if(!me.channels)
        	send_reply(target, "-+- Channel list is empty");
        else if(!chan_prefix(cp[0]))
                send_reply(target, "-+- Incorrect channel name (Bad prefix (%s))", cp);
        else if(strlen(cp) > CHAN_LEN)
                send_reply(target, "-+- Channel name too long (%d chars)", strlen(cp));
        else if((i = get_index_by_chan_f(cp)) == -1)
                send_reply(target, "-+- Channel '%s' not found", cp);
	else if(!strcasecmp(cp, KNB_ALPHABET) && strcasecmp(to, KNB_ALPHABET) != 0 && strlen(me.nick) == 1 && me.catched == true)
		send_reply(target, "-+- You cannot %s `%s' (Cannot leave us with letter!)", cmd, cp);
        else
        {
            if(!strcasecmp(cmd, "rejoin") || !strcasecmp(cmd, "cycle"))
            {
                send_reply(target, "-+- Rejoining '%s'", cp);
                me.channel[i].status = 1;
                sock_write(me.sock, "PART %s :%s\r\n", me.channel[i].name_f, me.reason);
                sock_write(me.sock, "JOIN %s %s\r\n", me.channel[i].name, strlen(me.channel[i].key) ? me.channel[i].key : "");
                add_penalty(8);
            }
            else
            {
		/* !j */
            	send_reply(target, "-+- Channel '%s' removed", cp);
            	// jezeli jestesmy na kanale lub w trakcie wchodzenia
        	if(me.channel[i].status)
            	{
            	    sock_write(me.sock, "PART %s :%s\r\n", me.channel[i].name_f, me.reason);
            	    add_penalty(5);
            	}
        	rem_channel(cp);
	    }
        }
        return;
    }
    if(!strcasecmp(cmd, "i") || !strcasecmp(cmd, "invite") || !strcasecmp(cmd, "op") || !strcasecmp(cmd, "dop") || !strcasecmp(cmd, "deop") ||
        !strcasecmp(cmd, "v") || !strcasecmp(cmd, "voice") || !strcasecmp(cmd, "devoice") || !strcasecmp(cmd, "dv") || !strcasecmp(cmd, "k") ||
        !strcasecmp(cmd, "kick") || !strcasecmp(cmd, "b") || !strcasecmp(cmd, "ban") || !strcasecmp(cmd, "ub") || !strcasecmp(cmd, "unban"))
    {
        int i;

	char *cp = NULL;
        char *np = NULL;

        if(strlen(args))
        {
        	if(strlen(arg[1]))
        	{
        		cp = (char *) arg[0];
        		np = (char *) arg[1];
                }
                else
                {
                	if(!strcmp(me.nick, to))
                	{
                		cp = (char *) arg[0];
                		np = n;
                	}
                	else
                	{
                		cp = to;
                	        np = (char *) arg[0];
                	}
                }
        }
        else
        {
        	if(strcmp(me.nick, to))
        	{
        		cp = to;
			np = n;
		}
		else
		{
			np = n;
		}
        }

        if(cp == NULL || np == NULL)
        {
        	if(!strcasecmp(cmd, "i") || !strcasecmp(cmd, "invite"))
            		send_reply(target, "-+- Usage: !%s <channel> [nick]", cmd);
            	else
            		send_reply(target, "-+- Usage: !%s [channel] [nick]", cmd);
        }
        else if(strlen(cp) > CHAN_LEN)
            send_reply(target, "-+- Channel name '%s' too long (%d chars)", cp, strlen(cp));
        else if(!chan_prefix(cp[0]))
            send_reply(target, "-+- Incorrect channel prefix (%s)", cp);
        else if((i = get_index_by_chan_f(cp)) == -1)
            send_reply(target, "-+- Channel '%s' not found on my list", cp);
        else if(me.channel[i].status != 2)
            send_reply(target, "-+- I'm not on that channel (%s)", cp);
        else
        {
            if(!strcasecmp(cmd, "i") || !strcasecmp(cmd, "invite"))
            {
                sock_write(me.sock, "INVITE %s %s\r\n", np, cp);
                add_penalty(4);
            }
            else if(!strcasecmp(cmd, "k") || !strcasecmp(cmd, "kick"))
            {
            	sock_write(me.sock, "KICK %s %s :Kicked by %s: %s\r\n", cp, np, n, me.reason);
            	add_penalty(4);
            }
            else if(!strcasecmp(cmd, "b")  || !strcasecmp(cmd, "ban"))
            {
                if(!extendhost(np, MASK_LEN, buf))
                {
                	send_reply(target, "-+- Incorrect host '%s'", np);
                }
                else
                {
             		sock_write(me.sock, "MODE %s +b %s\r\n", cp, buf);
                	add_penalty(4);
                }
            }
            else if(!strcasecmp(cmd, "ub")  || !strcasecmp(cmd, "unban"))
            {
                if(!extendhost(np, MASK_LEN, buf))
                {
                	send_reply(target, "-+- Incorrect host '%s'", np);
                }
                else
                {
             		sock_write(me.sock, "MODE %s -b %s\r\n", cp, buf);
                	add_penalty(4);
                }
            }

            else
            {
                if(!strcasecmp(cmd, "op") || !strcasecmp(cmd, "v") || !strcasecmp(cmd, "voice"))
                {
                    sock_write(me.sock, "MODE %s +%c %s\r\n", cp, !strcasecmp(cmd, "op") ? 'o' : 'v', np);
                    add_penalty(4);
                }
                else
                {
                    sock_write(me.sock, "MODE %s -%c %s\r\n", cp, (!strcasecmp(cmd, "dop") || !strcasecmp(cmd, "deop")) ?  'o' : 'v', np);
                    add_penalty(4);
                }

            }
        }
        return;
    }

    if(!strcasecmp(cmd, "uptime") || !strcasecmp(cmd, "up"))
    {
    	int day, hour, min, sec, rest;

	day = (int) (now - me.start) / (3600*24);
    	rest = (int) (now - me.start) % (3600*24);

	hour = rest / 3600;
	rest %= 3600;

	min = rest / 60;
	rest %= 60;

	sec = rest;

	if(day && hour && min && sec)
	    sprintf(buf, "Uptime: %d day%s, %d hour%s, %d min%s, %d sec%s",
			day, (day == 1) ? "" : "s", hour, (hour == 1) ? "" : "s", min,
			(min == 1) ? "" : "s", sec, (sec == 1) ? "" : "s");
	if(day && hour && min && !sec)
		    sprintf(buf, "Uptime: %d day%s, %d hour%s, %d min%s",
			day, (day == 1) ? "" : "s", hour, (hour == 1) ? "" : "s", min,
			(min == 1) ? "" : "s");
	if(day && hour && !min && sec)
		    sprintf(buf, "Uptime: %d day%s, %d hour%s, %d sec%s",
			day, (day == 1) ? "" : "s", hour, (hour == 1) ? "" : "s",
			sec, (sec == 1) ? "" : "s");
	if(day && hour && !min && !sec)
		    sprintf(buf, "Uptime: %d day%s, %d hour%s",
			day, (day == 1) ? "" : "s", hour, (hour == 1) ? "" : "s");
	if(day && !hour && min && sec)
		    sprintf(buf, "Uptime: %d day%s, %d min%s, %d sec%s",
			day, (day == 1) ? "" : "s", min,
			(min == 1) ? "" : "s", sec, (sec == 1) ? "" : "s");
	if(day && !hour && min && !sec)
		    sprintf(buf, "Uptime: %d day%s, %d min%s",
			day, (day == 1) ? "" : "s", min,
			(min == 1) ? "" : "s");
	if(day && !hour && !min && sec)
		    sprintf(buf, "Uptime: %d day%s, %d sec%s",
			day, (day == 1) ? "" : "s",
			sec, (sec == 1) ? "" : "s");
	if(day && !hour && !min && !sec)
		    sprintf(buf, "Uptime: %d day%s",
			day, (day == 1) ? "" : "s");
	if(!day && hour && min && sec)
		    sprintf(buf, "Uptime: %d hour%s, %d min%s, %d sec%s",
			 hour, (hour == 1) ? "" : "s", min,
			(min == 1) ? "" : "s", sec, (sec == 1) ? "" : "s");
	if(!day && hour && min && !sec)
		    sprintf(buf, "Uptime: %d hour%s, %d min%s",
			 hour, (hour == 1) ? "" : "s",
			 sec, (sec == 1) ? "" : "s");
	if(!day && hour && !min && sec)
		    sprintf(buf, "Uptime: %d hour%s, %d sec%s",
			hour, (hour == 1) ? "" : "s",
			sec, (sec == 1) ? "" : "s");
	if(!day && hour && !min && !sec)
		    sprintf(buf, "Uptime: %d hour%s",
			hour, (hour == 1) ? "" : "s");
	if(!day && !hour && min && sec)
		    sprintf(buf, "Uptime: %d min%s, %d sec%s",
			 min, (min == 1) ? "" : "s", sec, (sec == 1) ? "" : "s");
	if(!day && !hour && min && !sec)
		    sprintf(buf, "Uptime: %d min%s",
			min, (min == 1) ? "" : "s");
	if(!day && !hour && !min && sec)
		    sprintf(buf, "Uptime: %d sec%s",
			sec, (sec == 1) ? "" : "s");

    	send_reply(target, "-+- %s", buf);
    	return;
    }    
    if(!strcasecmp(cmd, "ver") || !strcasecmp(cmd, "version"))
    {
	send_reply(target, "-+- %s (%s) v%s by %s (%s)", KNB_SHORT, KNB_FULL, KNB_VERSION_STR, KNB_AUTOR, KNB_EMAIL);
	return;    
    }
    if(!strcasecmp(cmd, "hello"))
    {
	send_reply(target, "world");
	return;
    }
    if(!strcasecmp(cmd, "save"))
    {
	if(save_uf(me.userfile))
	    send_reply(target, "-+- Userfile saved to file '%s'", me.userfile);
	else
	    send_reply(target, "-+- Cannot save userfile to file '%s': %s", me.userfile, strerror(errno));
	return;
    }
    if(!strcasecmp(cmd, "backup"))
    {
	char b[CHAN_LEN + 2];
	sprintf(b, "~%s", me.userfile); 
	
	if(save_uf(b))
	    send_reply(target, "-+- Userfile backuped to file '%s'", b);
	else
	    send_reply(target, "-+- Cannot backup userfile to file '%s';%s", b, strerror(errno));
	return;
    }
    if(!strcasecmp(cmd, "keepnick") || !strcasecmp(cmd, "catch"))
    {
	if(!strlen(args))
	    send_reply(target, "-+- Catch mode is '%s'", me.catch ? "on" : "off");
	else
	{
	    if(!isbool(arg[0]))
		send_reply(target, "-+- Usage: !%s [on | off]", cmd);
	    else
	    {
		me.catch = get_bool(arg[0]);
		send_reply(target, "-+- Catch mode set to '%s'", me.catch ? "on" : "off");
	    }
	}
	return;
    }
    if(!strcasecmp(cmd, "reconnect") || !strcasecmp(cmd, "r"))
    {
	me.reconnect = true;
	send_quit("Reconnecting");
	return;
    }
    if(!strcasecmp(cmd, "s") || !strcasecmp(cmd, "server") 
#ifdef HAVE_IPV6
     || !strcasecmp(cmd, "s6") || !strcasecmp(cmd, "server6")
#endif
     )
    {
	int i, j, t;
	if(!strlen(args))
	{	
	    send_reply(target, "-+- Usage: !%s <server> [port]", cmd);
	    return;
	}
	else
	{
	    if(strlen(arg[0]) > MASK_LEN)
	    {
		send_reply(target, "-+- 1st argument too long for '%s' (%d chars)", cmd, strlen(arg[0]));
		return;
	    }
	    
#ifdef HAVE_IPV6 
	    if(!strcasecmp(cmd, "s6") || !strcasecmp(cmd, "server6"))
		t = 6; 
	    else
#endif
		t = 4;		

	    for(i = 0, j = -1; i < me.servers && j == -1; i++)
		if(!strcasecmp(me.server[i].name, arg[0]) && me.server[i].type == t)
		    j = i;
	
	    if(strlen(arg[1]))
	    {
		if(strlen(arg[1]) > 5)
		{
		    send_reply(target, "-+- 2nd argument too long for '%s' (%d chars)", cmd, strlen(arg[1]));
		    return;
		}
		
		if(!_isnumber(arg[1]))
		{
		    send_reply(target, "-+- Conflicting types in 2nd argument for '%s' (%s)", cmd, arg[1]);
		    return;
		}
		i = atoi(arg[1]);
	    }
	    else
	    {
		if(j == -1)
		    i = 6667;
		else
		    i = me.server[j].port;
	    }

	    
	    if(j != -1)
	    {
		strcpy(me.custom_server.ip, me.server[j].ip);
		
		jump:
		    me.reconnect = true; 
		    me.last_server = -1; 
		    me.custom_server.port = i; 
		    me.custom_server.type = t; 
		    strcpy(me.custom_server.name, arg[0]); 
		    
		    send_quit("Changing server");
		return;
	    }
	    else
	    {
		char buf[MAX_LEN];
		struct hostent *h;
		
#ifdef HAVE_IPV6
		if(t == 6)
		    h = gethostbyname2(arg[0], PF_INET6);
		else
#endif
		    h = gethostbyname(arg[0]);
		    
		if(!h)
		{
		    send_reply(target, "-+- Unknown host for '%s' (%s)", cmd, arg[0]);
		    return; 
		}    
		
#ifdef HAVE_IPV6
		if(t == 6)
		    strcpy(me.custom_server.ip, (char *) inet_ntop(PF_INET6, h->h_addr, buf, MAX_LEN));
		else
#endif		
		    strcpy(me.custom_server.ip, (char *) inet_ntop(PF_INET, h->h_addr, buf, MAX_LEN));		
		
		goto jump;
	    }
	}
	return;
    }
    if(!strcasecmp(cmd, "die") || !strcasecmp(cmd, "q") || !strcasecmp(cmd, "quit") || !strcasecmp(cmd, "exit") || !strcasecmp(cmd, "kill") || !strcasecmp(cmd, "disco") || !strcasecmp(cmd, "disconnect") || !strcasecmp(cmd, "shutdown"))
    {
	save_uf(me.userfile);
	send_quit("Killed by %s: %s", n, me.reason);
	exit(0);
    }
    if(!strcasecmp(cmd, "restart") || !strcasecmp(cmd, "reboot"))
    {
	save_uf(me.userfile);
	send_quit("Restarting: %s", me.reason);
	unlink(me.pidfile);
	sprintf(buf, "%s %s", me.program, me.conf);
	int i = system(buf);
	exit(0);    
    }
    if(!strcasecmp(cmd, "ison") || !strcasecmp(cmd, "check"))
    {
	int i, j;
	char b[MIN_LEN];
	
	memset(b, 0, sizeof(b));
	
	for(i = 0, j = 0; i < me.letters; i++)
	    if(!me.letter[i].status || me.letter[i].status == 2)
	    {
		strcat(b, " ");
		strcat(b, me.letter[i].nick);
		strcat(b, ",");
		j++;
	    }
	
	if(!j) 
	    strcpy(b, " none");
	else
	    b[strlen(b) - 1] = 0;
	        
	if(!me.catched) send_reply(target, "-+- Offline(%d):%s", j, b);
	else send_reply(target, "-+- Offline(%d):%s (outdated)", j, b); 
	return;
    }	
    if(!strcasecmp(cmd, "tempnicks"))
    {
	int i, j;
	char b[MIN_LEN];
	
	memset(b, 0, sizeof(b));
    
	for(i = 0, j = 0; i < me.letters; i++)
	    if(istemp_letter(i))
	    {
		strcat(b, " ");
		strcat(b, me.letter[i].nick);
		strcat(b, ",");
		j++;
	    }
	    
	if(!j)
	    strcpy(b, " none");
	else // usuwamy ostatni zbedny przecinek
	    b[strlen(b) - 1] = 0;
	    
	send_reply(target, "-+- Tempnicks(%d):%s", j, b);
	return;
    }
    if(!strcasecmp(cmd, "+tempnick"))
    {
	int i;
	
	if(!strlen(args))
	    send_reply(target, "-+- Usage: !%s <nick>", cmd);
	else if(strlen(arg[0]) > NICK_LEN)
	    send_reply(target, "-+- Nick '%s' is too long (%d chars)", arg[0], strlen(arg[0]));
	else if(me.letters == MAX_NICKS)
	    send_reply(target, "-+- Nicklist is full");
	else
	{	
	    if((i = get_index_by_letter(arg[0])) != -1)
	    {
		if(istemp_letter(i))
		    send_reply(target, "-+- Tempnick '%s' allready exists", arg[0]);
		else
		    send_reply(target, "-+- Nick '%s' allready exists (In config file)", arg[0]);
	    }
	    else
	    {
		me.letter[me.letters].temp = true;
		me.letter[me.letters].status = 0;
		me.letter[me.letters].time = 0;
		strcpy(me.letter[me.letters++].nick, arg[0]);
		
		send_reply(target, "-+- Added '%s' to nicklist", arg[0]);
	    }
	}
	return;
    }
    if(!strcasecmp(cmd, "-tempnick"))
    {
	int i, j;
	
	if(!strlen(args))
	    send_reply(target, "-+- Usage: !%s <nick>", cmd);
	else if(strlen(arg[0]) > NICK_LEN)
	    send_reply(target, "-+- Nick '%s' is too long (%d chars)", arg[0], strlen(arg[0]));
	else if(!me.letters)
	    send_reply(target, "-+- Nicklist is empty");
	else
	{
	    if((i = get_index_by_letter(arg[0])) == -1)
		send_reply(target, "-+- Nick '%s' not found", arg[0]);
	    else
	    {
		if(!istemp_letter(i))
		    send_reply(target, "-+- Nick '%s' isnt temporary", arg[0]);
		else
		{
		    for(j = i; i + 1 < me.letters; i++)
		    {
			me.letter[i].status = me.letter[i + 1].status;
			me.letter[i].temp = me.letter[i + 1].temp;
			me.letter[i].time = me.letter[i + 1].time;
			strcpy(me.letter[i].nick, me.letter[i + 1].nick);
			
			if(i + 2 > me.letters) // czyscymy ostatnia pozycje
			{
			    memset(me.letter[i].nick, 0, sizeof(me.letter[i].nick));
			    me.letter[i].status = 0;
			    me.letter[i].time = 0;
			    me.letter[i].temp = false;
			}
		    }
		    me.letters--;
		    send_reply(target, "-+- Nick '%s' removed", arg[0]);
		}
	    }
	}
	return;
    }
    if(!strcasecmp(cmd, "channels") || !strcasecmp(cmd, "chans"))
    {
	int i;
	
	if(!me.channels)
	    send_reply(target, "-+- Channel list is empty");
	else
	    for(i = 0; i < me.channels; i++)
		send_reply(target, "-+- [%2d] %s %s", i+1, me.channel[i].name, strlen(me.channel[i].key) ? me.channel[i].key : "");
	return;    
    }
    
    if(!strcasecmp(cmd, "nicks"))
    {
	int i, j;
	char b[MIN_LEN];
	
	memset(b, 0, sizeof(b));
	
	for(i = 0, j = 0; i < me.letters; i++)
	    if(!istemp_letter(i))
	    {
		strcat(b, " ");
		strcat(b, me.letter[i].nick);
		strcat(b, ",");
		j++;
	    }
	    
	if(!j)
	    strcpy(b, " none");
	else
	    b[strlen(b) - 1] = 0;
	    
	send_reply(target, "-+- Nicks(%d):%s", j, b);
	return;         
    }
    if(!strcasecmp(cmd, "n") || !strcasecmp(cmd, "nick"))
    {
	if(!strlen(args))
	    send_reply(target, "-+- Usage: !%s [-yes] <nick>", cmd);
	else 
	{
	    if(!strlen(arg[1]))
	    {
		if(strlen(arg[0]) > NICK_LEN)
		    send_reply(target, "-+- Nick '%s' is too long (%d chars)", arg[0], strlen(arg[0]));
		else
		{    
		    if(me.catched)
			send_reply(target, "-+- Cannot do that! But if You really want to change nick do !%s -yes %s", cmd, arg[0]);
		    else
		    {
			if(isdigit((int) me.nick[0]) && isdigit((int) arg[0][0]))
			    send_reply(target, "-+- My nick is allready my uid");
			else
			{    
			    if(!add_queue(arg[0], HIGH_PRIO))
				send_reply(target, "-+- Cannot add to queue (It's full). Try again later");
			}
		    }
		}
	    }
	    else
	    {
		if(strcasecmp(arg[0], "-yes") && strcasecmp(arg[0], "yes") && strcasecmp(arg[0], "--yes"))
		    send_reply(target, "-+- Usage: !%s [-yes] <nick>", cmd);
		else if(strlen(arg[1]) > NICK_LEN)
		    send_reply(target, "-+- Nick '%s' is too long (%d chars)", arg[1], strlen(arg[1]));
		else
		{    
		    if(isdigit((int) me.nick[0]) && isdigit((int) arg[1][0]))
		        send_reply(target, "-+- My nick is allready my uid");
		    else
		    {    
		        if(!add_queue(arg[1], HIGH_PRIO))
		    	    send_reply(target, "-+- Cannot add to queue (It's full). Try again later");
    		    }
		}

	    }
	}
	return;
    }
    if(!strcasecmp(cmd, "update") || !strcasecmp(cmd, "u"))
    {
	if(strlen(args))
	{
	    if(!strcmp(arg[0], "--with-restart") || !strcmp(arg[0], "-r") || !strcmp(arg[0], "-with-restart") || !strcmp(arg[0], "restart") || !strcmp(arg[0], "--with-reboot") || !strcmp(arg[0], "-with-reboot") || !strcmp(arg[0], "reboot") || !strcmp(arg[0], "--r") || !strcmp(arg[0], "r"))
	    {
		me.update.restart = true;
	    }	
	    else
	    {
		send_reply(target, "-+- Usage: !%s [--with-restart]", cmd);
		return;
	    }
	}
	else
	{
	    me.update.restart = false;
	    strcpy(me.update.target, n);
	}
	
	fork_and_go();
	return;
    }    
    if(!strcasecmp(cmd, "reload") || !strcasecmp(cmd, "rehash"))
    {
	int i;
	char b[CHAN_LEN+2];
	
	sprintf(b, "~%s", me.userfile);
	
	if(!save_uf(b))
	{
#ifdef DEBUG
	    printd("-+- Cannot backup userfile: %s", strerror(errno));
#endif
	    send_reply(target, "-+- Cannot backup userfile: %s", strerror(errno));
	    return;
	}
	// clearing user list 
	for(user = first_user; user; user2 = user, user = user->next)
	{	
		if(user == first_user)
		    first_user = first_user->next;
		else
		    user2->next = user->next;
		free(user);
	}		
	first_user = NULL;
	
	if(me.channels)
	    for(i = me.channels; i > 0; i--)
	    {
		sock_write(me.sock, "PART %s :Reloading\r\n", me.channel[i - 1].name);
		rem_channel(me.channel[i - 1].name);
		add_penalty(5);	
	    }
	
	if(me.letters)
	{    
    	    i = me.letters - 1;    
	    while(istemp_letter(i))    
	    {
		me.letter[i].status = 0;
		me.letter[i].temp = false;
		me.letter[i].time = 0;
		memset(me.letter[i].nick, 0, sizeof(me.letter[i].nick));
		i--;
		me.letters--;
	    }
	}
	
	if(!load_uf(me.userfile))
	{
	    if(!load_uf(b))
	    {
		send_reply(target, "-+- Cannot load userfile backup: %s", strerror(errno));
		send_quit("Reload falied");
		exit(6);
	    }
	    send_reply(target, "-+- Cannot load userfile: %s", strerror(errno));
	    return;
	}
	unlink(b); // removing backup uf 
	send_reply(target, "-+- Userfile reloaded");
	channels();
	return;
    }	
    if(!strcasecmp(cmd, "msg") || !strcasecmp(cmd, "m") || !strcasecmp(cmd, "notice") || !strcasecmp(cmd, "privmsg"))
    {
	int i;
	char *tar, *msg = NULL;
	char *bp;
	bool test;
	
	for(i = 0, test = false; i < strlen(args) && test == false; i++)
	    if(args[i] == ' ')
		test = true;
	
	if(test)
	{
	    bp = args;
    	    while(*bp != ' ' && *bp)
		*bp++;
	    *bp++ = 0;
	
	    msg = bp;
	}
	tar = args;
	
	if(!strlen(args))
	    send_reply(target, "-+- Usage: !%s <target> <message>", cmd);
	else if(msg == NULL)
	    send_reply(target, "-+- Message not given for '%s'", cmd);
	else if(strlen(tar) > CHAN_LEN) 
	    send_reply(target, "-+- Target is too long (%d chars)", strlen(target));
	else
	    send_reply(tar, msg);

	return;
    }
#ifdef FLOOD_CMD  /* stupid */
    if(!strcasecmp(cmd, "msgflood") || !strcasecmp(cmd, "mmsg") || !strcasecmp(cmd, "mflood") || !strcasecmp(cmd, "mprivmsg"))
    {
	int i;
	char *tar, *msg = NULL;
	char *bp;
	bool test;
	
	for(i = 0, test = false; i < strlen(args) && test == false; i++)
	    if(args[i] == ' ')
		test = true;
	
	if(test)
	{
	    bp = args;
    	    while(*bp != ' ' && *bp)
		*bp++;
	    *bp++ = 0;
	
	    msg = bp;
	}
	tar = args;
	
	if(!strlen(args))
	    send_reply(target, "-+- Usage: !%s <target> <message>", cmd);
	else if(msg == NULL)
	    send_reply(target, "-+- Message not given for '%s'", cmd);
	else if(strlen(tar) > CHAN_LEN) 
	    send_reply(target, "-+- Target is too long (%d chars)", strlen(target));
	else
	{
	    for(i = 0; i < 10; i++)
		send_privmsg(tar, msg);
	    add_penalty(20); // just in case ;p
	}
	return;
    }    
#endif
    if(!strcasecmp(cmd, "say"))
    {
	if(!strlen(args))
	    send_reply(target, "-+- Usage: !%s <message>", cmd);
	else if(!strcmp(to, me.nick))
	    send_reply(n, args);
	else 
	    send_reply(to, args);
	return;
    }    	
    if(!strcasecmp(cmd, "help"))
    {
	send_reply(target, "-+- Go to http://%s%s and RFTM/RTFS", UP_HTTP_HOST, UP_HTTP_PATH);
	return;
    }    
    /*
    if(!strcasecmp(cmd, "x")) // crash test :)
    {
	char *x = NULL;
	send_reply(target, "-+- bye, bye ;(");
	*x = 666; 
    } */
}
