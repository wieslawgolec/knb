#include "prots.h"

void end_update()
{
    struct stat info;
    
    if(me.update.parent) close(me.update.parent);
    if(me.update.child) close(me.update.child);
    
    me.update.size = 0;
    me.update.version_int = 0;
    me.update.child = 0;
    me.update.pid = 0;
    me.update.parent = 0;
    
    memset(me.update.version_str, 0, sizeof(me.update.version_str));
    
    if(!stat(UP_INFOFILE, &info))
	unlink(UP_INFOFILE);
    if(!stat(UP_LOCKFILE, &info))
	unlink(UP_LOCKFILE);
}

bool check_update()
{
    struct stat info;
    
    if(me.update.child > 0 || !stat(UP_LOCKFILE, &info))
	return true;
    return false;
}

bool set_variable3(char *var, char *args, char *error)
{
        if(!strcmp(var, UP_VERSION_STR)) 
        {
        	if(!strlen(args))
        		return err_return(-102, error, var, NULL);

		if(strlen(args) > CHAN_LEN)
			return err_return(1, error, var, args);

		if(strlen(me.update.version_str))
                    return true;

		strcpy(me.update.version_str, args);
		return true;
        }
        if(!strcmp(var, UP_VERSION_INT)) 
        {
        	if(!strlen(args))
        		return err_return(-102, error, var, NULL);

		if(strlen(args) > CHAN_LEN)
			return err_return(1, error, var, args);

		if(me.update.version_int)
                    return true;
		if(!_isnumber(args))
		    return err_return(-109, error, var, args);
		    
		me.update.version_int = (long int) atoi(args);
		return true;
        }
        if(!strcmp(var, UP_FREEBSD_STR) || !strcmp(var, UP_LINUX_STR) || !strcmp(var, UP_SUNOS_STR) || !strcmp(var, UP_WINDOWS_STR))    // 2 arguments
        {	
		int i;
        	if(!strlen(args))
        		return err_return(-102, error, var, NULL);

		if(strlen(args) > CHAN_LEN)
			return err_return(1, error, var, args);
		if(!_isnumber(args))
		    return err_return(-109, error, var, args);

		if(me.update.size)
		    return true;
		    
		if(!strcmp(var, UP_FREEBSD_STR)) i = 1;
		else if(!strcmp(var, UP_LINUX_STR)) i = 2;
		else if(!strcmp(var, UP_SUNOS_STR)) i = 3;
		else if(!strcmp(var, UP_WINDOWS_STR)) i = 4;
		else i = 0;
	
		if(!me.uname_int || me.uname_int != i)
		    return true;    
		
		me.update.size = (off_t) atoi(args);
		return true;
        }
	if(!strcmp(var, UP_SRC_STR)) /* po prostu pomijamy tez wpisy zwracajac prawde */
	    return true;
	    
	return err_return(666, error, var, NULL); // unknown variable
}

bool load_info(char *file)
{
	int i, e, j;
	FILE *f;
	char *var, *arg, *bp;
	char buf[MAX_LEN];
	char error[MIN_LEN];

        if(!(f = fopen(file, "r")))
        {	
        	printf("-+- Cannot open infofile: %s\n", strerror(errno));
		return false;
        }
        e = 0;
	i = 0;
	j = 0;
	while(!feof(f))
	{
		i++;
		memset(buf, 0, sizeof(buf));
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

                if(!set_variable3(var, arg, error))
                {
#ifdef DEBUG
                	printf("%s:%d: error: %s\n", file, i, error);
#endif
                	e++;
                }
	}
	
	fclose(f);

	if(!me.update.size)
	{
#ifdef DEBUG
	    printf("%s: error: package size not found\n", file);
#endif
	    e++;
	}
	if(!me.update.version_int || !strlen(me.update.version_str))
	{
#ifdef DEBUG
	    printf("%s: error: version info not found\n", file);
#endif
	    e++;
	}

	if(e)
	{
#ifdef DEBUG
		printf("-+- Falied to load infofile (Too many errors (%d))\n", e);
#endif
		exit(1);
		return false;
	}
	else
	{
#ifdef DEBUG
		printf("-+- Infofile loaded\n");
#endif
	        return true;
	}
}

bool fetch_info(char *file)
{	
    int i;    
    char buf[MIN_LEN];
    struct stat info;
    
    sprintf(buf, "%s/%s", UP_HTTP_PATH, UP_INFOFILE);
    
    if(!(i = getfile(UP_INFOFILE, UP_INFOFILE))) /* ok */
    {
	if(!stat(file, &info))
	{
	    printf("-+- Checking for new releases\n");
	    if(load_info(file))
	    {
		unlink(file);
		return true;
	    }
	    unlink(file);
	    return false;
	}
	printf("-+- Cannot stat file '%s': %s\n", file, strerror(errno)); 
	return false;
    }
    else
    {
	printf("-+- Fetching falied (Error: %d)\n", i * (-1));
	return false;
    }
}

bool fetch_package()
{
    int i;
    char buf[MIN_LEN];
    struct stat info;
    
    sprintf(buf, "knb-%s-%s.tar.gz", me.update.version_str, me.uname_str);
    
    printf("-+- Downloading '%s'\n", buf);

    if(!(i = getfile(buf, UP_TARFILE))) /*ok */
    {
	if(!stat(UP_TARFILE, &info)) /* plik istnieje? */
	{
	    if(info.st_size !=  me.update.size)
	    {
		printf("-+- File '%s' is corrupted (Size missmatch)\n", UP_TARFILE);
#ifdef DEBUG
		printf("-+- update: %ld, infile: %ld\n", (long int) info.st_size, (long int) me.update.size);
#endif
		
		unlink(UP_TARFILE);
		return false;
	    }
#ifdef DEBUG
	    printf("-+- File '%s' saved as: '%s'\n", buf, UP_TARFILE);
#endif
	    return true;    
	}    
	printf("-+- Cannot stat file '%s': %s\n", UP_TARFILE, strerror(errno));
	return false;
    }
    else
    {
	printf("-+- Falied (Error: %d)\n", i * (-1));
	return false;
    }        
}

void upgrade(char *file)
{
    char buf[MIN_LEN];
    int ret;
    
    printf("-+- Upgrading %s to version %s\n", KNB_SHORT, me.update.version_str);

    ret = unlink("knb"); // usuwamy stary symlink
    
    sprintf(buf, "tar -zxf %s", file);
    
    ret = system(buf); //rozpakowuje
    ret = unlink(file); // usuwam archiwum
    
    sprintf(buf, "cp knb/knb-%s-%s knb-%s-%s", me.update.version_str, me.uname_str, me.update.version_str, me.uname_str);
    ret = system(buf); //kopiuje binarke
    
    ret = system("rm -fr knb/"); // usuwam katalog
    
    sprintf(buf, "knb-%s-%s", me.update.version_str, me.uname_str);
    ret = symlink(buf, "knb");
    
    printf("-+- Your %s is now up2date ;>\n", KNB_SHORT);
}

bool preupdate()
{
    char buf[MAX_LEN];
    struct hostent *h;
    
    if((h = gethostbyname(me.update.name)))
    {
	strcpy(me.update.ip, (char *) inet_ntop(PF_INET, h->h_addr, buf, MAX_LEN));
#ifdef DEBUG
	printf("-+- Host '%s' resolved to '%s'\n", me.update.name, me.update.ip);
#endif
	return true;
    }
    printf("-+- Unknown host: '%s'\n", me.update.name);
    return false;
}

bool update()
{
    /* unbuffered streams */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    
    if(!preupdate())
    {
	return false;
    }	
    
    if(fetch_info(UP_INFOFILE))
    {
	if((long int) atoi(KNB_VERSION_INT) >= me.update.version_int)
	{
	    printf("-+- Your %s is allready up2date ;>\n", KNB_SHORT);	
	    return false;
	}
	if(fetch_package())
	{
	    upgrade(UP_TARFILE);
	    return true;
	}
	unlink(UP_TARFILE);
	return false;
    }
    return false;
}

bool fork_and_go()
{
    FILE *f; /* lock file */
    int tmp[2]; /* for pipe */
    
    if(check_update())
    {
	send_notice(me.update.target, "-+- Allready updating");
	return false;
    }
    
    if((f = fopen(UP_LOCKFILE, "w")))
    {
	fclose(f);
    }
    else
    {
	send_notice(me.update.target, "-+- Cannot create update lockfile: %s", strerror(errno));
	end_update();
	return false;
    }
    
    if(pipe(tmp))
    {
	send_notice(me.update.target, "-+- Pipe() falied: %s", strerror(errno));
	end_update();
	return false;
    }

    me.update.child = tmp[0];
    me.update.parent = tmp[1];

    switch((me.update.pid = fork()))
    {
	case -1: 
		send_notice(me.update.target, "-+- Fork() falied: %s", strerror(errno));
		end_update();
		return false;
	case 0:
		if(!fcntl(me.update.parent, F_SETFL, O_NONBLOCK))
		{
		    /* rd */
		    dup2(me.update.parent, fileno(stdout));
		    dup2(me.update.parent, fileno(stderr));
		    if(update() && me.update.restart)
		    {
			sleep(1);
			kill(me.update.pid, SIGUSR1);
		    }
		    else
		    {
			sleep(1);
			kill(me.update.pid, SIGUSR2);    
		    }
		}
//		end_update();
		exit(0);
		return true;
	default:
	    return true;	
    }
    return false;
}

int get_index_by_uname(char *dupa)
{
    if(!strcmp(dupa, "freebsd"))
	return 1;
    if(!strcmp(dupa, "linux"))
	return 2;
    if(!strcmp(dupa, "sunos"))
	return 3;
    if(!strcmp(dupa, "cygwin"))
	return 4;    
    
    return 0;
}
