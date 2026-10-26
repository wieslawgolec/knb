#include "prots.h"

bool set_variable2(char *var, char *args, char *error)
{
	int i, j;
	char param[MAX_NICKS][MIN_LEN];

        if(!strcmp(var, UF_TEMPNICK)) // 40
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

                        strcpy(me.letter[me.letters].nick, param[a]);
                        me.letter[me.letters++].temp = true;
		}
		return true;
        }
        if(!strcmp(var, UF_CHANNEL))    // 2 arguments
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
                {
                	if(me.channel[me.channels].cfg)
               			return err_return(-103, error, var, param[0]);
               		else
               		{
				if(!strcmp(me.channel[j].name, param[0]))
				{
					strcpy(me.channel[j].name, param[0]);
					strcpy(me.channel[j].name_f, param[0]);
				}
				if(strlen(param[1]))
				{
					if(!strcmp(me.channel[j].key, param[1]))
						strcpy(me.channel[j].key, param[1]);
				}
				else
				{
					if(strlen(me.channel[j].key))
						strcpy(me.channel[j].key, "");
				}
				me.channel[j].status = 0;
				me.channel[j].cfg = true;
				return true;
               		}
                }
		strcpy(me.channel[me.channels].name, param[0]);
		strcpy(me.channel[me.channels].name_f, param[0]);
		strcpy(me.channel[me.channels].key, strlen(param[1]) ? param[1] : "");
		me.channel[me.channels].cfg = true;
		me.channel[me.channels++].status = 0;
		return true;
        }
        if(!strcmp(var, UF_USERMASK))  // 1 argument
        {
        	char buf[MASK_LEN+1];
        	char n[MASK_LEN+1], u[MASK_LEN+1], h[MASK_LEN+1];
		
		memset(buf, 0, sizeof(buf));
		memset(n, 0, sizeof(n));
		memset(u, 0, sizeof(u));
		memset(h, 0, sizeof(h));
		
         	if(!(i = strlen(args)))
         		return err_return(-102, error, var, NULL);
         	if(i > MASK_LEN)
         		return err_return(0, error, var, args);

		if(!extendhost(args, MASK_LEN, buf))
			return err_return(-112, error, var, args);
			
		if((user = first_user))
		{
			if(check_flags(buf))
				return err_return(-103, error, var, args);
		}
                get_nuh(buf, n, u, h);

		if(strlen(n) > NICK_LEN)
		    return err_return(-113, error, var, args);
		if(strlen(u) > IDENT_LEN)
		    return err_return(-114, error, var, args);
                if(strlen(h) > MASK_LEN)
		    return err_return(-115, error, var, args);

		user = first_user;
		
		while(user && user->next)
			user = user->next;

    		user2 = malloc(sizeof(struct knb_userlist));
    		if(first_user)
        		user->next = user2;
    		else
        		first_user = user2;

    		strncpy(user2->nick, n, NICK_LEN);
		strncpy(user2->ident, u, IDENT_LEN);
		strncpy(user2->host, h, MASK_LEN);

    		user2->next = NULL;
		
		return true;
        }
	return err_return(666, error, var, NULL); // unknown variable
}

bool load_uf(char *file)
{
	int i, e, j;
	FILE *f;
	char *var, *arg, *bp;
	char buf[MAX_LEN];
	char error[MID_LEN];

        if(!(f = fopen(file, "r")))
        {
        	printf("-+- Cannot open userfile %s: %s\n", file, strerror(errno));
		return false;
        }
        e = 0;
	i = 0;
	j = 0;
		
	while(!feof(f))
	{
		i++;
		memset(buf, 0, sizeof(buf));
		memset(error, 0, sizeof(error));
                var = fgets(buf, MAX_LEN, f);
                trim(buf);
		if(strlen(buf))
		    buf[strlen(buf) - 1] = '\0';

	        if(buf[0] == '\0' || buf[0] == '#' || buf[0] == ';' || buf[0] == '\n')
                	continue;

		bp = buf;
                while(*bp != ' ' && *bp)
                	bp++;
                *bp++ = 0;
                var = buf;
                arg = bp;

                if(!set_variable2(var, arg, error))
                {
                	printf("%s:%d: error: %s\n", file, i, error);
                	e++;
                }
                else
                {
                 	if(!strcmp(var, UF_USERMASK))
                 		j++;
                }
	}
	
	fclose(f);

	if(!(user = first_user))
	{
	    printf("%s: warning: no such masks\n", file);
	}
	if(!me.channels)
	{
	    printf("%s: warning: no such channels\n", file);
	}

	if(e)
	{
		printf("-+- Userfile not loaded (Too many errors (%d))\n", e);
		exit(1);
		return false;
	}
	else
	{

		printf("-+- Userfile loaded (%d mask%s)\n", j, (j == 1) ? "" : "s");
	        return true;
	}
}

bool save_uf(char *file)
{
	int i, j;
	FILE *f;

        if(!(f = fopen(file, "w")))
        {
        	printf("-+- Cannot open userfile %s: %s\n", file, strerror(errno));
		return false;
        }

#ifdef DEBUG
      	fprintf(f, "# DEBUG MODE\n");
#endif
        fprintf(f, "# %s (%s) v%s by %s (%s)\n", KNB_SHORT, KNB_FULL, KNB_VERSION_STR, KNB_AUTOR, KNB_EMAIL);
        fprintf(f, "# %s", ctime(&now));

	if((user = first_user))
	{
		fprintf(f, "\n");
	    	while(user)
	    	{
			fprintf(f, "%s %s!%s@%s\n", UF_USERMASK, user->nick, user->ident, user->host);
			user = user->next;
	    	}
        }

	if(me.channels)
	{
		fprintf(f, "\n");

		for(j = 0; j < me.channels; j++)
		{
	    		fprintf(f, "%s %s %s\n", UF_CHANNEL, me.channel[j].name, strlen(me.channel[j].key) ? me.channel[j].key : "");
		}
	}
	if(me.letters)
	{
		for(i = 0, j = 0; i < me.letters; i++)
			if(istemp_letter(i))
			{
				if(!j) fprintf(f, "\n");

				fprintf(f, "%s %s\n", UF_TEMPNICK, me.letter[i].nick);
				j++;
			}
		if(j) fprintf(f, "\n");
	}
	fclose(f);
#ifdef DEBUG
	printd("Userfile saved\n");
#endif
	return true;
}

