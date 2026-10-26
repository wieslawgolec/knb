#include "prots.h"

/* add penalty to a bot like ircd does */
void add_penalty(int t)
{
    if(me.penalty < now)
	me.penalty = now;
    me.penalty += t;
}

/* check bot penalty */
int has_penalty(void)
{
    if(me.penalty < now)
	me.penalty = now;
    return (me.penalty - now);
}

/* notice send */
void send_notice(char *to, char *format, ...)
{
    va_list args;
    char s[MIN_LEN];

    va_start(args, format);
    vsprintf(s, format, args);
    va_end(args);

    sock_write(me.sock, "NOTICE %s :%s\r\n", to, s);
    add_penalty(3);
}

/* privmsg send */
void send_privmsg(char *to, char *format, ...)
{
    va_list args;
    char s[MIN_LEN];

    va_start(args, format);
    vsprintf(s, format, args);
    va_end(args);

    sock_write(me.sock, "PRIVMSG %s :%s\r\n", to, s);
    me.idle = now;
    add_penalty(3);
}

/* reply: 0 = notice, 1 = privmsg */
void send_reply(char *to, char *format, ...)
{
    va_list args;
    char s[MIN_LEN];

    va_start(args, format);
    vsprintf(s, format, args);
    va_end(args);

    if(me.reply_type)
    {
	sock_write(me.sock, "PRIVMSG %s :%s\r\n", to, s);
	me.idle = now;
    }
    else
	sock_write(me.sock, "NOTICE %s :%s\r\n", to, s);	
	
    add_penalty(3);
}

void send_quit(char *msg, ...)
{
	va_list args;
	char s[MIN_LEN];

    	va_start(args,msg);
    	vsprintf(s, msg, args);
    	va_end(args);

    	if(me.conn == 3) sock_write(me.sock, "QUIT :%s\r\n", s);
    	kill_socket(me.sock);
    	me.sock = 0;
    	me.conn = 0;
	prepare_channels();
}

bool check_flags(char *mask)
{
    char n[NICK_LEN], u[IDENT_LEN], h[MASK_LEN];
    get_nuh(mask, n, u, h);

    for(user = first_user; user; user = user->next)
	if(!ircd_match(user->nick, n)  &&
	!ircd_match(user->ident, u) &&
	!ircd_match(user->host, h))
	    return true;

    return false;
}
void get_nick(char *who, char *n)
{
    char *p = who;
    if(who[0] == ':')
	who++;
    while(*p != '!' && *p)
	p++;
    *p = 0;
    strcpy(n, who);
}

void get_nuh(char *who, char *n, char *u, char *h)
{
    char *ex = strchr(who, '!');
    char *at = strchr(who, '@');

    if(who[0] == ':')
	who++;
    
    strncpy(n, who, (int) abs(who - ex));
    strncpy(u, ex+1, (int) abs(at - ex) - 1);
    strcpy(h, at+1);
    
    n[(int) abs(who - ex)] = 0;
    u[(int) abs(at - ex) - 1] = 0;
}
    

bool extendhost(char *host, unsigned int len, char *buf)
{
    char *ex, *at;

    if(strlen(host) + 10 > len || !isrealstr(host) || *host == '#')
	 return false;

    ex = strchr(host, '!');
    at = strchr(host, '@');

    if(ex != strrchr(host, '!') || at != strrchr(host, '@'))
	return false;

    if(at)
    {
        if(!ex)
        {
            if(at == host)
		strcpy(buf, "*!*");
            else
		strcpy(buf, "*!");
            strcat(buf, host);
        }
        else if(ex == host)
        {
            strcpy(buf, "*");
            strcat(buf, host);
        }
        else
		strcpy(buf, host);
        if(*(at + 1) == '\0')
		strcat(buf, "*");
        return true;
    }
    else
    {
        if(ex)
		return false;
        if(strchr(host, '.') || strchr(host, ':'))
        {
            strcpy(buf, "*!*@");
            strcat(buf, host);
            return true;
        }
        strcpy(buf, "*!");
        strcat(buf, host);
        strcat(buf, "@*");
        return true;
    }
}

void generate_nick(char *nick)
{
	int nicklen, applen;
	char *n;

	nicklen = strlen(nick);
	applen = strlen(me.nickappend);

	if(nicklen >= NICK_LEN && (!strchr(me.nickappend, nick[NICK_LEN - 1]) || nick[NICK_LEN - 1] == me.nickappend[applen - 1]))
	{
		nick[0] = 0;
		return;
	}
	n = strchr(me.nickappend, nick[nicklen - 1]);

	if(n)
	{
		if(nick[nicklen - 1] == me.nickappend[rand() % applen]) nick[nicklen] = me.nickappend[0];
		else nick[nicklen - 1] = me.nickappend[abs(n - me.nickappend) + 1];
	}
	else nick[nicklen] = me.nickappend[0];
	nick[strlen(nick)] = 0;
}

void rand_nick(char *nick)
{
    int l, i, n;

    char let1[] = "qwrtpsfghjklzxvbnm";
    char let2[] = "euioay";

    int let1len = strlen(let1);
    int let2len = strlen(let2);

    n = rand() % NICK_LEN + 1;
    l = rand() % 2;

    for(i = 0; i < n; i++)
    {
	if(l)
	{
	    nick[i] = let1[rand() % let1len];
	    l = 0;
	}
	else
	{
	    nick[i] = let2[rand() % let2len];
	    l = 1;
	}
    }
    nick[n + 1] = 0;
}

/* netwrok praser */
void irc(void)
{
    int i, j;

    char b[MIN_LEN + 1];

    loop()
    {
	j = -1;

	if(strlen(me.buf))
	    for(i = 0; me.buf[i]; i++)
		if(me.buf[i] == '\n')
		{
		    j = i;
		    break;
		}

	if(j == -1)
	    return;

	memset(b, 0, sizeof(b));
	strncpy(b, me.buf, j - 1);
	irc_msg(b);
	strcpy(me.buf, (char *) &me.buf[j + 1]);
    }
}

/* main ircd message praser */
void irc_msg(char *b)
{
    int i, j, k, l;

    char *s[20];

    for(i = 0; i < 20; i++)
	s[i] = NULL;
    k = j = 0;

    loop()
    {
	l = 0;
	for(i = j; ; i++)
	    if(b[i] == ' ' || !b[i])
	    {
	    /*
		// possible sigsegv
		if(j != 0 && b[j] == '[')
		{
		    j++;
		    i = strlen(b);
		}
	    */
		if(j != 0 && b[j] == ':')
		{
		    j++;
		    i = strlen(b);
		}
		s[k] = malloc(i - j + 1);
		strncpy((char *) &s[k][0], (char *) &b[j], i - j);
		s[k][i - j] = 0;
		j = i + 1;
		l = 1;
		k++;
		break;
	    }
	if(!l || !b[i])
	    break;
    }

    me.timeout = now;

    if(!strcmp(s[1], "020"))
    {
	me.conn = 2;
#ifdef DEBUG
	printd("Connection in progress");
#endif
	if(strlen(me.logfile))
		loguj("Connection in progress");
    }
    if(!strcmp(s[1], "042"))
    {
	strcpy(me.uid, s[3]);

	if(!strlen(me.nick) || !strcmp(me.nick, "0"))
	    strcpy(me.nick, me.uid);
#ifdef DEBUG
	printd("Got uid: '%s'", me.uid);
#endif
	if(strlen(me.logfile))
		loguj("Got uid: '%s'", me.uid);
    }

    if(!strcmp(s[1], "001"))
    {
	char *p = strstr(s[3], s[2]);

	add_penalty(3);
	me.connected = now;
	me.idle = now;
	me.conn = 3;

	if(p != NULL && strchr(p, '!') != NULL && strchr(p, '@') != NULL)
	{
	    char n[NICK_LEN+1], u[IDENT_LEN+1], h[MASK_LEN+1];
	    get_nuh(p, n, u, h);

	    if(strcmp(me.nick, n)) strcpy(me.nick, n);
	    if(strcmp(me.ident, u)) strcpy(me.ident, u);
	    if(strcmp(me.host, h)) strcpy(me.host, h);
#ifdef DEBUG
	    printd("Got welcome: '%s!%s@%s'", me.nick, me.ident, me.host);
#endif
	    if(strlen(me.logfile))
	    	loguj("Got welcome: '%s!%s@%s'", me.nick, me.ident, me.host);

	    me.next_chan_check = now + CHAN_CHECK_DELAY;
	    me.next_penalty_reset = now + PENALTY_RESET_TIME; /* 24 h*/
	    channels();	
	}    
	else
	{
#ifdef DEBUG
	    printd("Got error: '%s' is not an ircnet server!", s[0] + 1);
#endif
	    if(strlen(me.logfile))
	    	loguj("Got error: '%s' is not an ircnet server!", s[0] + 1);

	    send_quit("%s is not an ircnet server!", s[0] + 1);
	}
    }
    if(!strcmp(s[0], "PING"))
    {
	add_penalty(2);
	sock_write(me.sock, "PING %s\r\n", s[1]);
    }
    if(!strcmp(s[1], "JOIN"))
    {
#ifdef DEBUG
        printd("Got join: '%s' '%s'", s[0]+1, s[2]);
#endif
	if(strlen(me.logfile))
		loguj("Got join: '%s' '%s'", s[0]+1, s[2]);

	got_join(s[0]+1, s[2]);
    }
    if(!strcmp(s[1], "PART"))
    {
#ifdef DEBUG
        printd("Got part: '%s' '%s' (%s)", s[0]+1, s[2], s[3]);
#endif
        if(strlen(me.logfile))
        	loguj("Got part: '%s' '%s' (%s)", s[0]+1, s[2], s[3]);
    }
    if(!strcmp(s[1], "KICK"))
    {
#ifdef DEBUG
        printd("Got kick: '%s' from '%s' by '%s' (%s)", s[3], s[2], s[0]+1, s[4]);
#endif
        if(strlen(me.logfile))
        	loguj("Got kick: '%s' from '%s' by '%s' (%s)", s[3], s[2], s[0]+1, s[4]);
	got_kick(s[0]+1, s[3], s[2]);
    }

    /* cannot join channel: limit, invite, banned, key */
    if(!strcmp(s[1], "471") || !strcmp(s[1], "473") || !strcmp(s[1], "474") || !strcmp(s[1], "475"))
    {
	i = get_index_by_chan_f(s[3]);
#ifdef DEBUG
	if(!strcmp(s[1], "471"))
      		printd("Cannot join '%s' (%s)", s[3], "Its full");
      	if(!strcmp(s[1], "473"))
      		printd("Cannot join '%s' (%s)", s[3], "Invite only");
	if(!strcmp(s[1], "474"))
      		printd("Cannot join '%s' (%s)", s[3], "You are banned");
      	if(!strcmp(s[1], "475"))
      		printd("Cannot join '%s' (%s)", s[3], "Key required");
#endif
	if(strlen(me.logfile))
	{
		if(!strcmp(s[1], "471"))
      			loguj("Cannot join '%s' (%s)", s[3], "Its full");
      		if(!strcmp(s[1], "473"))
      			loguj("Cannot join '%s' (%s)", s[3], "Invite only");
		if(!strcmp(s[1], "474"))
      			loguj("Cannot join '%s' (%s)", s[3], "You are banned");
      		if(!strcmp(s[1], "475"))
      			loguj("Cannot join '%s' (%s)", s[3], "Key required");
      	}
	if(i != -1)
	    me.channel[i].status = 0;
    }
    if(!strcmp(s[1], "QUIT"))
    {
#ifdef DEBUG
      	printd("Got quit: '%s' (%s)", s[0]+1, s[2]);
#endif
        if(strlen(me.logfile))
        	loguj("Got quit: '%s' (%s)", s[0]+1, s[2]);
        got_quit(s[0]+1, s[2]);
    }
    if(!strcmp(s[1], "303"))
    {
#ifdef DEBUG
	printd("Got ison: '%s'", s[3]);

	if(strlen(me.logfile))
	    loguj("Got ison: '%s'", s[3]);

#endif
    	got_ison(s[3]);
    }
    if(!strcmp(s[1], "INVITE"))
    {
#ifdef DEBUG
      	printd("Got invite: from '%s' to '%s'", s[0]+1, s[3]);
#endif
	if(strlen(me.logfile))
		loguj("Got invite: from '%s' to '%s'", s[0]+1, s[3]);

	got_invite(s[0]+1, s[3]);
    }
    if(!strcmp(s[1], "PRIVMSG"))
    {
	if(s[3][0] == '\001' && s[3][strlen(s[3]) - 1] == '\001')
	{
	    s[3][strlen(s[3]) - 1] = '\0';
#ifdef DEBUG
      	    printd("Got ctcp: '%s' from %s", s[3]+1, s[0]+1);
#endif
	    if(strlen(me.logfile))
	    	loguj("Got ctcp: '%s' from %s", s[3]+1, s[0]+1);

	    got_ctcp(s[0]+1, s[3] + 1);
	}
	else
	{
	
#ifdef DEBUG
      	    printd("got msg: '%s' from %s", s[3]+1, s[0]+1);

	    if(strlen(me.logfile))
	    	loguj("got msg: '%s' from %s", s[3]+1, s[0]+1);
#endif
	    got_privmsg(s[0]+1, s[2], s[3]);
	}
    }
    if(!strcmp(s[1], "433")) // nick w uzyciu
    {
#ifdef DEBUG
        printd("Got fail: '%s' allready in use", s[3]);
#endif
	if(strlen(me.logfile))
		loguj("Got fail: '%s' allready in use", s[3]);

	got_fail(s[3], 1); // nick allready in use
    }
    if(!strcmp(s[1], "432")) // bad nick
    {
#ifdef DEBUG
        printd("Got fail: '%s' is incorrect", s[3]);
#endif
	if(strlen(me.logfile))
		loguj("Got fail: '%s' is incorrect", s[3]);

	got_fail(s[3], 3); // bad
    }
    if(!strcmp(s[1], "437")) // nick/channel chwilowo niedostepny
    {
    	i = get_index_by_chan_f(s[3]);
#ifdef DEBUG
	if(i == -1)
       		printd("Got fail: '%s' is temporality unavailable", s[3]);
       	else
       		printd("Cannot join '%s' (Temporality unavailable)", s[3]);
#endif
	if(strlen(me.logfile))
	{
		if(i == -1)
       			loguj("Got fail: '%s' is temporality unavailable", s[3]);
       		else
       			loguj("Cannot join '%s' (Temporality unavailable)", s[3]);
        }
        if(i == -1)
        	got_fail(s[3], 2); // nick unavailable in use
	else
		me.channel[i].status = 0;
    }
    if(!strcmp(s[1], "NICK"))
    {
#ifdef DEBUG
      	printd("Got nick: '%s' is now know as '%s'", s[0]+1, s[2]);
#endif
	if(strlen(me.logfile))
		loguj("Got nick: '%s' is now know as '%s'", s[0]+1, s[2]);
	got_nick(s[0]+1, s[2]);
    }
    if(!strcmp(s[1], "047")) // kolid :)
    {
#ifdef DEBUG
      	printd("Colide on '%s'", me.nick);
#endif
        if(strlen(me.logfile))
        	loguj("Colide on '%s'", me.nick);

	add_penalty(4);
/*
	if(add_queue(me.uid, HIGH_PRIO))
	{
	    i = get_index_by_queue(me.uid);
	    queue[i].send = true;
	}
*/
    }
    if(!strcmp(s[0], "ERROR"))
    {
#ifdef DEBUG
      	printd("Got disconnect: '%s'", s[1]);
#endif
	if(strlen(me.logfile))
		loguj("Got disconnect: '%s'", s[1]);
	me.conn = 0;
	send_quit("Error (%s)", s[1]);
    }
    for(i = 0; i < 20; i++)
	free(s[i]);
}

/* check input on socket */
void check_input(void)
{
    struct timeval t;
    fd_set fdr, fdw;

    int j = 0, k;
    bool bad;
    char buf[MAX_LEN];

    t.tv_sec = 1;
    t.tv_usec = 0;

    FD_ZERO(&fdr);
    FD_ZERO(&fdw);

    if(me.sock)
    {
	if(me.conn)
	    FD_SET(me.sock, &fdr);
	else
	    FD_SET(me.sock, &fdw);
	
	j = me.sock;
	
	if(now >= me.timeout + TIMEOUT_TIME)
	{
#ifdef DEBUG
            printd("Got disconnected: '%s!%s@%s' (Connection timeout)", me.nick, me.ident, me.host);
#endif
	    if(strlen(me.logfile))
	    	loguj("Got disconnected: '%s!%s@%s' (Connection timeout)", me.nick, me.ident, me.host);
		
	    send_quit("Connection timeout");
	}
    }

    if(me.update.child)
    {
	FD_SET(me.update.child, &fdr);
	if(me.update.child > me.sock)
	    j = me.update.child;
    }
  
    if(select(j+1, &fdr, &fdw, NULL, &t) < 1)
	return;

    if(me.sock)
    {
	/* update ? */
	if(me.update.child && FD_ISSET(me.update.child, &fdr))
	{
	    j = read(me.update.child, buf, MAX_LEN);
	    if(j > 0 && me.conn == 3)
		send_reply(me.update.target, "%s", buf);
	    else if(j == -1)
	    {
		end_update();
	    }
	}
	
        bad = false;
	if(FD_ISSET(me.sock, &fdr))
	{
	    k = j = strlen(me.buf);
	    if((k += read(me.sock, (char *) &me.buf[j], 1024 - j)) == -1)
	    {
#ifdef DEBUG
      		printd("Got disconnected: '%s!%s@%s' (Read error)", me.nick, me.ident, me.host);
#endif
		if(strlen(me.logfile))
		    loguj("Got disconnected: '%s!%s@%s' (Read error)", me.nick, me.ident, me.host);

		send_quit("Read error");
                bad = true;
	    }
	    else
	    {
		if(!bad)
		{
		    me.buf[k] = 0;
		    irc();
		}
	    }
	}
	if(FD_ISSET(me.sock, &fdw))
	{
	
	    int j, dupa;
	    socklen_t k = 4;

	    dupa = getsockopt(me.sock, SOL_SOCKET, SO_ERROR, (void *) &j, &k);

	    if(j || dupa == -1)
	    {
		bad = true;
#ifdef DEBUG
      		printd("Got disconnected: '%s!%s@%s' (Connection refused)", me.nick, me.ident, me.host);
#endif
		if(strlen(me.logfile))
		    loguj("Got disconnected: '%s!%s@%s' (Connection refused)", me.nick, me.ident, me.host);

		send_quit("Connection refused");
	    } 
	    if(!bad)
	    {
		if(me.conn < 1)
		{
		    me.conn = 1;
#ifdef DEBUG
		    printd("Connection with server estabilished");
#endif
		    if(strlen(me.logfile))
			loguj("Connection with server estabilished");

    		    if(strcasecmp(me.nick, me.orignick))
    		    	strcpy(me.nick, me.orignick);

		    sock_write(me.sock, "NICK %s\r\n", me.nick);
		    sock_write(me.sock, "USER %s 8 * :%s\r\n", me.user, me.realname);
		}
	    }

	}
    }
}

int get_index_by_chan(char *ch)
{
	int i, j;
	
	for(i = 0, j = -1; i < me.channels && j == -1; i++)
		if(!strcasecmp(me.channel[i].name, ch))
			j = i;

        return j;
}

int get_index_by_chan_f(char *ch)
{
	int i, j;
	char chfix[CHAN_LEN + 1];
	
	/* chan fixes for names with '!' */
	memset(chfix, 0, sizeof(chfix));
	
	if(ch[0] == '!')
	{
	    chfix[0] = '!';
	    for(i = 1, j = 0; i < strlen(ch) && i < CHAN_LEN; i++)
	    {
		if(++j > 5)
		{
		    chfix[strlen(chfix)] = ch[i];
		}
	    }
	}
	else
	    strncpy(chfix, ch, CHAN_LEN);
	
	for(i = 0, j = -1; i < me.channels && j == -1; i++)
		if(!strcasecmp(me.channel[i].name, chfix))
			j = i;

        return j;
}


int get_index_by_letter(char *nick)
{
	int i, j;

	for(i = 0, j = -1; i < me.letters && j == -1; i++)
		if(!strcasecmp(me.letter[i].nick, nick))
			j = i;

        return j;
}

bool istemp_letter(int index)
{
	if(me.letter[index].temp)
		return true;
	return false;
}

void selfcheck()
{
	if(now >= me.next_notice)
	{
    		me.next_notice = now + 60 + (rand() % 3601); // next notice every 1-61 mins
        	send_notice(me.nick, "%s", KNB_SHORT);
	}
}

void send_ison()
{
	char buf[MIN_LEN];
	int i;

	memset(buf, 0, sizeof(buf));

	for(i = 0; i < me.letters; i++)
	{
		strcat(buf, me.letter[i].nick);
        	if((i + 1) < me.letters)
        		strcat(buf, " ");
	}

	add_penalty(2+me.delay);
	sock_write(me.sock, "ISON %s\r\n", buf);
	me.ison = true;
}


bool add_channel(char *ch, char *key)
{
    int i;
    
    if(me.channels == MAX_CHANS) 
	return false;
    
    if((i = get_index_by_chan(ch)) != -1)
	return false;
    
    strcpy(me.channel[me.channels].name, ch);
    strcpy(me.channel[me.channels].name_f, ch);
    strcpy(me.channel[me.channels].key, key);
    me.channel[me.channels++].status = 0;
    return true;
}

bool rem_channel(char *ch)
{
    int i, j;

    if(!me.channels) return false;
    if((j = get_index_by_chan(ch)) == -1)
	return false;
    
    for(i = j; (i + 1) < me.channels; i++)
    {
	strcpy(me.channel[i].name, me.channel[i + 1].name);
	strcpy(me.channel[i].name_f, me.channel[i + 1].name_f);	
	strcpy(me.channel[i].key, me.channel[i + 1].key);
	me.channel[i].status = me.channel[i + 1].status;
	me.channel[i].cfg = me.channel[i + 1].cfg;
	if((i + 2) > me.channels)
	{    
	    memset(me.channel[i + 1].name, 0, sizeof(me.channel[i + 1].name));	    
	    memset(me.channel[i + 1].name_f, 0, sizeof(me.channel[i + 1].name_f));	    
	    memset(me.channel[i + 1].key, 0, sizeof(me.channel[i + 1].key));
	    me.channel[i + 1].status = 0;
	    me.channel[i + 1].cfg = false;
	}
    }
    me.channels--;
    return true;
}

bool clear_channels()
{
    if(!me.channels) return false;
    
    memset(me.channel, 0, sizeof(me.channel));
    me.channels = 0;
    return true;
}

void prepare_channels()
{
    int i;
    
    if(!me.channels) return;
    
    for(i = 0; i < me.channels; i++)
	me.channel[i].status = 0;
}

void channels()
{
    int i;
    
    if(!me.channels)
	return;

    if(has_penalty() > 10)
	return;

    // only add chanel if alphabet wasnt excluded in cfg
    if(strlen(me.nick) == 1 && me.alphabet == 1 && me.catched == true)
    {
        if((i = get_index_by_chan(KNB_ALPHABET)) == -1)
        {
            add_channel(KNB_ALPHABET, "");
        }
    }

    for(i = 0; i < me.channels && has_penalty() < 15; i++)
	if(!me.channel[i].status)
	{
	    add_penalty(3);
	    sock_write(me.sock, "JOIN %s %s\r\n", me.channel[i].name, me.channel[i].key);
	    me.channel[i].status = 1;
	}

}

void work()
{
    if(me.catch)
    {
	int i;
	if(!me.catched) ins_queue(); 
	
	if(me.send)
	    return;
	
	i = has_penalty();
	
	if(!queues)
	{
	    if(me.ison)
		return;
	    if(i < 3 && !me.catched)	
		send_ison();
	    return;	
	}
	else
	{
	    switch(queue[0].prio)
	    {
		case HIGH_PRIO:
		    if(i < 10) goto send;
		    else return;
		case MID_PRIO:
		    if(i < 7) goto send;
		    else return;
		case LOW_PRIO:
		    if(i < 4) goto send;
		    else return;
	    }
	    
	    send:
		add_penalty(3+me.delay);
		sock_write(me.sock, "NICK %s\r\n", isdigit((int) queue[0].nick[0]) ? "0" : queue[0].nick);
		queue[0].send = true;
		me.send = true;
	    return;
	}
    }
    else
    {
	selfcheck();
    }
}

void preconnect()
{
	me.catched = false;
	me.ison = false;
	me.send = false;
	clear_queue();
	prepare_channels();
	me.timeout = now;
	me.next_penalty_reset = now + PENALTY_RESET_TIME;
	if(me.sock)
	{
#ifdef DEBUG
	    printd("Connection attempt timed out");
#endif
	    if(strlen(me.logfile))
		loguj("Connection attempt timed out");
		
	    send_quit("Connection attempt timed out");
	}
}
