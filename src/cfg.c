#include "prots.h"

bool err_return(int number, char *error, char *var, char *param)
{
        switch(number)
        {
        	/* too long letters/nicks */
        	case 0: sprintf(error, "argument too long for '%s' (%s)", var, param); break;
        	case 1: sprintf(error, "%dst argument too long for '%s' (%d chars)", number, var, (int) strlen(param)); break;
        	case 2: sprintf(error, "%dnd argument too long for '%s' (%d chars)", number, var, (int) strlen(param)); break;
        	case 3: sprintf(error, "%drd argument too long for '%s' (%d chars)", number, var, (int) strlen(param)); break;
        	case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14:
        	case 15: case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23: case 24: case 25:
		case 26: case 27: case 28: case 29: case 30: case 31: case 32: case 33: case 34: case 35: case 36:
        	case 37: case 38: case 39:
        	case 40: sprintf(error, "%dth argument too long for '%s' (%d chars)", number, var, (int) strlen(param)); break;
		/* overrange */
        	case -1: sprintf(error, "overrange in argument for '%s' (%s)", var, param); break;
        	case -2: sprintf(error, "overrange in %dst argument for '%s' (%s)", (number - 1) * (-1), var, param); break;
        	case -3: sprintf(error, "overrange in %dnd argument for '%s' (%s)", (number - 1) * (-1), var, param); break;
        	case -4: sprintf(error, "overrange in %drd argument for '%s' (%s)", (number - 1) * (-1), var, param); break;
        	case -5:
        	case -6: sprintf(error, "overrange in %dth argument for '%s' (%s)", (number - 1) * (-1), var, param); break;

		/* others */
        	case -101: sprintf(error, "list is full for '%s' (%s)", var, param); break;
        	case -102: sprintf(error, "too few arguments for '%s'", var); break;
        	case -103: sprintf(error, "duplicate entries for '%s' (%s)", var, param); break;
        	case -104: sprintf(error, "conflicting types in argument for '%s' (%s)", var, param); break;
                case -105: sprintf(error, "conflicting types in %dst argument for '%s' (%s)", ((number * (-1)) - 104), var, param); break;
                case -106: sprintf(error, "conflicting types in %dnd argument for '%s' (%s)", ((number * (-1)) - 104), var, param); break;
                case -107: sprintf(error, "conflicting types in %drd argument for '%s' (%s)", ((number * (-1)) - 104), var, param); break;
                case -108:
                case -109: sprintf(error, "conflicting types in %dth argument for '%s' (%s)", ((number * (-1)) - 104), var, param); break;
                case -110: sprintf(error, "unknown host for '%s' (%s)", var, param); break;
                case -111: sprintf(error, "unknown prefix for '%s' (%s)", var, param); break;
                case -112: sprintf(error, "incorrect mask for '%s' (%s)", var, param); break;
                case -113: sprintf(error, "nick too long for '%s' (%s)", var, param); break;
                case -114: sprintf(error, "ident too long for '%s' (%s)", var, param); break;
                case -115: sprintf(error, "host too long for '%s' (%s)", var, param); break;
		case -116: sprintf(error, "sysname mismatch for '%s' (%s)", var, param); break;

                default: sprintf(error, "no such variable '%s'", var);
        }
 	return false;
}

bool set_variable(char *var, char *args, char *error)
{
	int i, j;
	char param[MAX_NICKS][MIN_LEN];

        if(!strcmp(var, CFG_NICKS)) // 40 arguemnts
        {
        	int a;

        	if(!strlen(args))
        		return err_return(-102, error, var, NULL);
                if(me.letters == MAX_NICKS)
                	return err_return(-101, error, var, args);

		str2words(param[0], args, MAX_NICKS, MIN_LEN);

		for(a = 0; a < MAX_NICKS && strlen(param[a]); a++)
		{
			if(strlen(param[a]) > NICK_LEN)
				return err_return(a + 1, error, var, param[a]);

			for(i = 0, j = -1; i < me.letters && j == -1; i++)
				if(!strcasecmp(me.letter[i].nick, param[a]))
					j = i;
			if(j != -1)
                        	return err_return(-103, error, var, param[j]);

                        strcpy(me.letter[me.letters++].nick, param[a]);
		}
		return true;
        }
	if(!strcmp(var, CFG_SERVER)
#ifdef HAVE_IPV6
	 || !strcmp(var, CFG_SERVER6)
#endif
	 )    // 2 arguments
	{
		char buf[MAX_LEN];
		struct hostent *h;
		int t;

                if(!strlen(args))
			return err_return(-102, error, var, NULL);

		if(me.servers == MAX_SERVERS)
			return err_return(-101, error, var, args);

    
		str2words(param[0], args, 2, MIN_LEN);

                if(strlen(param[0]) > MASK_LEN)
			return err_return(1, error, var, param[0]);
#ifdef HAVE_IPV6
		if(!strcmp(var, CFG_SERVER6))
		    t = 6;
		else
#endif
		    t = 4;

                for(i = 0, j = -1; i < me.servers && j == -1; i++)
			if(!strcasecmp(param[0], me.server[i].name) &&
				me.server[i].type == t)
				j = i;
		if(j != -1)
			return err_return(-103, error, var, param[0]);

                j = 0;

                if((i = strlen(param[1])))
                {
			if(i > 5)
				return err_return(2, error, var, param[1]); /* too long */

			if(!_isnumber(param[1]))
				return err_return(-106, error, var, param[1]); /* conflicting types */
			j = atoi(param[1]);

			if(j < 1 || j > 65535)
				return err_return(-3, error, var, param[0]);  /* overrange */
		}
		else
		{
                	j = 6667;
                }
		
#ifdef HAVE_IPV6
		if(t == 6)
		    h = gethostbyname2(param[0], PF_INET6);
		else
#endif		
		    h = gethostbyname(param[0]);
		    
                if(!(h)) 
                	return err_return(-110, error, var, param[0]);  /* unknown host */

		strcpy(me.server[me.servers].name, param[0]);
#ifdef HAVE_IPV6
                if(t == 6) strcpy(me.server[me.servers].ip, inet_ntop(PF_INET6, h->h_addr, buf, MAX_LEN));
		else
#endif
		    strcpy(me.server[me.servers].ip, inet_ntop(PF_INET, h->h_addr, buf, MAX_LEN));

		me.server[me.servers].port = j;
                me.server[me.servers++].type = t;
                return true;
        }
        if(!strcmp(var, CFG_CHANNEL))    // 2 arguments
        {
        	if(!strlen(args))
        		return err_return(-102, error, var, NULL);
                if(me.channels == MAX_CHANNELS)
                	return err_return(-101, error, var, args);

        	str2words(param[0], args, 2, MIN_LEN);

        	if(strlen(param[0]) > CHAN_LEN)
        		return err_return(1, error, var, param[0]);

        	if(strlen(param[1]) > CHAN_LEN)
        		return err_return(1, error, var, param[1]);

		if(!chan_prefix(param[0][0]))
			return err_return(-111, error, var, param[0]);

                for(i = 0, j = -1; i < me.channels && j == -1; i++)
                	if(!strcasecmp(me.channel[i].name, param[0]))
                		j = i;

                if(j != -1)
               		return err_return(-103, error, var, param[0]);

		strcpy(me.channel[me.channels].name, param[0]);
		strcpy(me.channel[me.channels].name_f, param[0]);
		strcpy(me.channel[me.channels].key, strlen(param[1]) ? param[1] : "");
		me.channel[me.channels].cfg = false;
		me.channel[me.channels++].status = 0;
		return true;
        }
        if(!strcmp(var, CFG_NICK))  // 1 argument
        {
         	if(!(i = strlen(args)))
         		return err_return(-102, error, var, NULL);
         	if(i > NICK_LEN)
         		return err_return(0, error, var, args);

		if(strlen(me.orignick))
			return err_return(-103, error, var, args);

         	strcpy(me.orignick, args);
                return true;
        }
	if(!strcmp(var, CFG_NICKAPPEND))
	{
         	if(!(i = strlen(args)))
         		return err_return(-102, error, var, NULL);
         	if(i > NICKAPPEND_LEN)
         		return err_return(0, error, var, args);

		if(strlen(me.nickappend))
			return err_return(-103, error, var, args);

         	strcpy(me.nickappend, args);
                return true;

	}
        if(!strcmp(var, CFG_LOGFILE) || !strcmp(var, CFG_PIDFILE) || !strcmp(var, CFG_USERFILE))
        {
         	if(!(i = strlen(args)))
         		return err_return(-102, error, var, NULL);
         	if(i > CHAN_LEN)
         		return err_return(0, error, var, args);

         	if(!strcmp(var, CFG_LOGFILE))
         	{
			if(strlen(me.logfile))
				return err_return(-103, error, var, args);

         		strcpy(me.logfile, args);
         	}
         	if(!strcmp(var, CFG_PIDFILE))
         	{
			if(strlen(me.pidfile))
				return err_return(-103, error, var, args);

        		strcpy(me.pidfile, args);
         	}
         	if(!strcmp(var, CFG_USERFILE))
         	{
			if(strlen(me.userfile))
				return err_return(-103, error, var, args);

         		strcpy(me.userfile, args);
         	}
                return true;
	}
        if(!strcmp(var, CFG_ALTNICK) || !strcmp(var, CFG_NOALTNICK))
        {
        	if(!(i = strlen(args)))
        		return err_return(-102, error, var, NULL);
        	if(i > 5)
                	return err_return(0, error, var, args);
                if(!isbool(args))
                	return err_return(-104, error, var, args);
                if(!strcmp(var, CFG_ALTNICK))
		{
		    if(me.altnick.set)
                	    return err_return(-103, error, var, args);

            	    me.altnick.var = get_bool(args);
		    me.altnick.set = true;
		}
		else
		{
		    if(me.noaltnick.set)
                	    return err_return(-103, error, var, args);

            	    me.noaltnick.var = get_bool(args);
		    me.noaltnick.set = true;		
		}
                return true;
        }
        if(!strcmp(var, CFG_IDENT))
        {
        	if(!(i = strlen(args)))
        		return err_return(-102, error, var, NULL);
        	if(i > IDENT_LEN)
                	return err_return(0, error, var, args);
		if(strlen(me.ident))
			return err_return(-103, error, var, args);
		strcpy(me.ident, args);
		return true;
        }
        if(!strcmp(var, CFG_VHOST))
        {
        	if(!(i = strlen(args)))
        		return err_return(-102, error, var, NULL);
        	if(i > MASK_LEN)
                	return err_return(0, error, var, args);
                if(strlen(me.vhost.ip))
                	return err_return(-103, error, var, args);

                strcpy(me.vhost.ip, args);
                return true;
        }
        if(!strcmp(var, CFG_REALNAME))
        {
        	if(!(i = strlen(args)))
        		return err_return(-102, error, var, NULL);
        	if(i > REALNAME_LEN)
        		return err_return(0, error, var, args);
        	if(strlen(me.realname))
        		return err_return(-103, error, var, args);

        	strcpy(me.realname, args);
                return true;
        }
	if(!strcmp(var, CFG_CTCP))
	{
	    if(!(i = strlen(args)))
		return err_return(-102, error, var, NULL);
	    if(i > 5)
		return err_return(0, error, var, args);
	    if(!_isnumber(args))
		return err_return(-104, error, var, args);
	    if(me.ctcp != -1)
		return err_return(-103, error, var, args);
	    i = atoi(args);
	    
	    if(i > 2 || i < 0)
		return err_return(-1 ,error, var, args);
	    me.ctcp = i;
	    return true;
	}
	if(!strcmp(var, CFG_REPLYTYPE))
	{
	    if(!(i = strlen(args)))
		return err_return(-102, error, var, NULL);
	    if(i > 7)
		return err_return(0, error, var, args);

	    if(!isreply(args))
                return err_return(-104, error, var, args);
		
	    if(me.reply_type != -1)
        	return err_return(-103, error, var, args);

	    me.reply_type = get_reply(args);
	    return true;	
	}
	if(!strcmp(var, CFG_REASON))
	{
	    if(!(i = strlen(args)))
		return err_return(-102, error, var, NULL);
	    if(i > REASON_LEN)
		return err_return(0, error, var, args);
	    if(strlen(me.reason))
		return err_return(-103, error, var, args);

	    strcpy(me.reason, args);
	    return true;
	}
	if(!strcmp(var, CFG_CTCPREPLY))
	{
	    if(!(i = strlen(args)))
		return err_return(-102, error, var, NULL);
	    if(i > CTCP_REPLY_LEN)
		return err_return(0, error, var, args);
	    if(strlen(me.ctcpreply))
		return err_return(-103, error, var, args);

	    strcpy(me.ctcpreply, args);
	    return true;
	}
	if(!strcmp(var, CFG_PUBLICREPLY))
	{
        	if(!(i = strlen(args)))
        		return err_return(-102, error, var, NULL);
        	if(i > 5)
                	return err_return(0, error, var, args);
                if(!isbool(args))
                	return err_return(-104, error, var, args);
		if(me.public_reply != -1)
                	return err_return(-103, error, var, args);

            	me.public_reply = get_bool(args);
                return true;
	}
	if(!strcmp(var, CFG_ALPHABET))
	{
	    if(!strlen(args))
	    {
        	if(!(i = strlen(args)))
        		return err_return(-102, error, var, NULL);
        	if(i > 5)
                	return err_return(0, error, var, args);
                if(!isbool(args))
                	return err_return(-104, error, var, args);
		if(me.alphabet != -1)
                	return err_return(-103, error, var, args);

            	me.alphabet = get_bool(args);
		return true;
	    }
	}
	if(!strcmp(var, CFG_DELAY))
	{
	    if(!(i = strlen(args)))
		return err_return(-102, error, var, NULL);
	    if(i > 5)
		return err_return(0, error, var, args);
	    if(!_isnumber(args))
		return err_return(-104, error, var, args);
	    if(me.delay != -1)
		return err_return(-103, error, var, args);
	    i = atoi(args);
	    
	    if(i > 10 || i < 0)
		return err_return(-1 ,error, var, args);
	    me.delay = i;
	    return true;
	}

#ifdef DEBUG
	if(!strcmp(var, CFG_DONTFORK))
	{
	    if(!strlen(args))
	    {
        	if(!(i = strlen(args)))
        		return err_return(-102, error, var, NULL);
        	if(i > 5)
                	return err_return(0, error, var, args);
                if(!isbool(args))
                	return err_return(-104, error, var, args);
		if(me.dontfork != -1)
                	return err_return(-103, error, var, args);

            	me.dontfork = get_bool(args);
                return true;
	    }
	}
#endif	
	return err_return(666, error, var, NULL); // unknown variable
}


bool load_cfg(char *file)
{
	int i, e;
	FILE *f;
	char *var, *arg, *bp;
	char buf[MAX_LEN];
	char error[MIN_LEN];
	
        if(!(f = fopen(file, "r")))
        {
        	printf("-+- Cannot open file %s: %s\n", file, strerror(errno));
		return false;
        }
        e = 0;
	i = 0;
	
	while(!feof(f))
	{
		i++;
		memset(buf, 0, sizeof(buf));
                var = fgets(buf, MAX_LEN, f);
		if(strlen(buf))
		    buf[strlen(buf) - 1] = '\0';

                trim(buf);

	        if(buf[0] == '\0' || buf[0] == '#' || buf[0] == ';' || buf[0] == '\n')
                	continue;

                /* extracting 1st word && res */
		bp = buf;
                while(*bp != ' ' && *bp)
                	bp++;
                *bp++ = 0;
                var = buf;
                arg = bp;

                if(!set_variable(var, arg, error))
                {
                	printf("%s:%d: error: %s\n", file, i, error);
                	e++;
                }
	}
	fclose(f);

	if(!strlen(me.orignick))
	{
	    printf("%s: error: mandatory option '%s' not set\n", file, CFG_NICK);
	    e++;
	}
	if(me.altnick.var && !strlen(me.nickappend))
	{
	    printf("%s: warning: optional option '%s' not set, using default\n", file, CFG_NICKAPPEND);
	    strcpy(me.nickappend, "abcdefghijklmnopqrstuvwxyz");
	}
	if(!me.servers)
	{
#ifdef HAVE_IPV6
	    printf("%s: error: mandatory option '%s' or '%s' not set\n", file, CFG_SERVER, CFG_SERVER6);
#else
	    printf("%s: error: mandatory option '%s' not set\n", file, CFG_SERVER);
#endif
	    e++;
	}
	if(!me.letters)
	{
	    printf("%s: error: mandatory option '%s' not set\n", file, CFG_NICKS);
	    e++;
	}

	if(e)
	{
		printf("-+- Config not loaded (Too many errors (%d))\n", e);
		exit(1);
		return false;
	}
	else
	{
		struct passwd *id = getpwuid(getuid());

		if(!id)
		{
		    if(!strlen(me.ident))
			strncpy(me.user, me.orignick, IDENT_LEN);
		    else
			strcpy(me.user, me.ident);
		}
		else
		    strncpy(me.user, id->pw_name, IDENT_LEN);

		if(!strlen(me.ident))
		    strcpy(me.ident, me.user);
		else
		    strcpy(me.user, me.ident);

		if(!strlen(me.userfile))
		    snprintf(me.userfile, CHAN_LEN, "%s.%s", me.orignick, USERFILE_EXT);

		if(!strlen(me.pidfile))
		    snprintf(me.pidfile, CHAN_LEN, "%s.%s", PIDFILE_EXT, me.orignick);

		strcpy(me.nick, me.orignick);
		
		if(me.ctcp == -1)
		    me.ctcp = 2; /* stability */
		if(!strlen(me.ctcpreply))
		    snprintf(me.ctcpreply, CTCP_REPLY_LEN, "%s (%s) v%s by %s (%s)\001",
		      KNB_SHORT, KNB_FULL, KNB_VERSION_STR, KNB_AUTOR, KNB_EMAIL);

		if(me.reply_type == -1)
		    me.reply_type = 0; /* notice as default reply*/
		
		if(me.public_reply == -1)
		    me.public_reply = 0; /* turning off public reply */

		if(!strlen(me.reason))
		    snprintf(me.reason, REASON_LEN, KNB_SHORT);
#ifdef DEBUG
		if(!strlen(me.logfile))
		    snprintf(me.logfile, CHAN_LEN, ".irc-log-%d", getpid());		

		if(me.dontfork == -1)
		    me.dontfork = 0;
#endif	
		if(me.alphabet == -1)
		    me.alphabet = 1;
		if(me.delay == -1)
		    me.delay = 0;

		printf("-+- Config loaded\n");
	        return true;
	}
}

