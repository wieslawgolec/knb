#include "prots.h"

void propaganda()
{
    printf("-+- %s (%s) v%s by %s (%s)\n", KNB_FULL, KNB_SHORT, KNB_VERSION_STR, KNB_AUTOR, KNB_EMAIL);
}

/* die with an error message */
void error(char *msg, ...)
{
    char s[MAX_LEN];
    va_list args;

    va_start(args, msg);
    vsprintf(s, msg, args);
    va_end(args);

    printf("-+- Error: %s\n", s);
    exit(0);
}

bool copy(char *from, char *to)
{
    int fdr, fdw, i, ret = 0;
    char buf[MAX_LEN];
    struct stat info;

    if((fdr = open(from, O_RDONLY)) < 0)
	return false;
    if(fstat(fdr, &info) == -1)
	return false;
    if((fdw = open(to, O_WRONLY | O_CREAT, info.st_mode)) < 0)
	return false;

    memset(buf, 0, sizeof(buf));
    while((i = read(fdr, buf, MAX_LEN)) > 0)
	if((ret = write(fdw, buf, i)) == -1)
	    break;

    close(fdr);
    close(fdw);
    return (ret == -1) ? false : true;
}

bool chan_prefix(char c)
{
    if(c == '#' || c == '!' || c == '&' || c == '+')
	return true;
    return false;
}

bool cmd_prefix(char c)
{
    if(c == '!' || c == '.' || c == '@')
	return true;
    return false;
}

void cron(char *prog, int i, char *argv[], int argc)
{
	char buf[MAX_LEN];
	struct stat s;
	int n;
	char *ret;

	FILE *p = popen("crontab -", "w");
	if(!p)
	{
		printf("-+- I dont have access to crontab (%s)\n", strerror(errno));
		return;
	}

	ret = getcwd(buf, MAX_LEN);
	for(n=0; i<argc; ++i)
	{
		if(stat(argv[i], &s) == 0)
		{
			fprintf(p, "*/10 * * * * cd %s; %s %s >/dev/null 2>&1\n", buf, prog, argv[i]);
			printf("-+- Adding: */10 * * * * cd %s; %s %s >/dev/null 2>&1\n", buf, prog, argv[i]);
			++n;
		}
		else printf("-+- Cannot stat `%s': %s\n", argv[i], strerror(errno));
	}

	if(!fclose(p))
	{
		printf("-+- Added %d %s%s to cron\n", n, KNB_SHORT, (n == 1) ? "" : "s");
		return;
	}
	else
	{
		printf("-+- Update failed. %s\n", strerror(errno));
		return;
	}
}

void str2words(char *word, char *str, int x, int y)
{
	int i, j;
	for(i=0; i<x; ++i)
	{
		while(isspace((int) *str))
		{
			if(*str == '\0') break;
			++str;
		}
		if(*str == '\0') break;

		for(j=0; j<y-1 && !isspace((int) *str); ++str)
		{
			if(*str == '\0') break;
			else
			{
				*word = *str;
				++word;
				++j;
			}
		}
		memset(word, 0, y - j - 1);

		word += y - j;
		if(*str == '\0') break;
	}
	for(++i; i<x; ++i)
	{
		memset(word, 0, y - 1);
		word += y;
	}
}

#ifdef DEBUG
/* print debug with time */
void printd(char *format, ...)
{
    va_list args;
    char s[MAX_LEN];
    struct tm *t;    
    
    if(me.dontfork)
    {
	time_t now = time(NULL);

	va_start(args, format);
	vsprintf(s, format, args);
	va_end(args);

	t = localtime(&now);
	printf("%02d:%02d:%02d -+- %s\n", t->tm_hour, t->tm_min, t->tm_sec, s);
    }
}
#endif

/* write log */
void loguj(char *format, ...)
{
    va_list args;
    char s[MIN_LEN];
    FILE *f;

    struct tm *czas = localtime(&now);

    va_start(args, format);
    vsprintf(s, format, args);
    va_end(args);
    f = fopen(me.logfile, "a+");

    if(f)
    {
	fprintf(f, "%02d:%02d:%02d -+- %s\n", czas->tm_hour, czas->tm_min, czas->tm_sec, s);
	fclose(f);
    }
#ifdef DEBUG
    else
    	printd("-+- Cannot open logfile %s: %s", me.logfile, strerror(errno));
#endif
}


/* write to a socket */
void sock_write(int fd, char *format, ...)
{
    va_list args;
    char s[MIN_LEN + 1];

    va_start(args, format);
    vsprintf(s, format, args);
    va_end(args);

    if((write(fd, &s, strlen(s))) == -1)
    {
	me.conn = 0;
	send_quit("Write error");
    }
}

int count(char *arr[])
{
    int i;

    for(i = 0; arr[i]; ++i);
	return i;
}

/* isnumber() wystepuje juz w ctype.h (freebsd) */
bool _isnumber(char *str)
{
    int i;

    for(i = 0; i < strlen(str); i++)
	if(!isdigit((int) str[i]))
	    return false;
    return true;
}

void kill_socket(int fd)
{
#ifdef _NO_LAME_ERRNO
    int e = errno;
#endif
    shutdown(fd, SHUT_RDWR);
    close(fd);
#ifdef _NO_LAME_ERRNO
    errno = e;
#endif
}

int do_connect(char *server, int port, char *vhost, int noblock)
{
    struct sockaddr_in sin;
    int s;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1)
        return -1;

    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    if(strlen(vhost))
        sin.sin_addr.s_addr = inet_addr(vhost);
    else
        sin.sin_addr.s_addr = INADDR_ANY;
    if(bind (s, (struct sockaddr *) &sin, sizeof (sin)) == -1)
    {
        kill_socket(s);
        return -1;
    }

    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(server);

    if(noblock == -1 && fcntl(s, F_SETFL, O_NONBLOCK) == -1)
    {
        kill_socket(s);
        return -1;
    }
    if(connect(s, (struct sockaddr *) &sin, sizeof(sin)) == -1)
    {
        if(noblock == -1 && errno == EINPROGRESS)
            return s;
        kill_socket(s);
        return -1;
    }
    if(noblock == 1 && fcntl(s, F_SETFL, O_NONBLOCK) == -1)
    {
        kill_socket(s);
        return -1;
    }
    return s;
}

#ifdef HAVE_IPV6
int do_connect6(char *server, int port, char *vhost, int noblock)
{
    struct sockaddr_in6 sin6;
    int s;

    s = socket(AF_INET6, SOCK_STREAM, 0);
    if(s == -1)
        return -1;

    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    if(strlen(vhost))
        inet_pton(AF_INET6, vhost, (void *) &sin6.sin6_addr);
    else
        sin6.sin6_addr = in6addr_any;

    if(bind (s, (struct sockaddr *) &sin6, sizeof (sin6)) == -1)
    {
        kill_socket(s);
        return -1;
    }

    memset(&sin6, 0, sizeof (struct sockaddr_in6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(port);
    inet_pton(AF_INET6, server, (void *) &sin6.sin6_addr);

    if(noblock == -1 && fcntl(s, F_SETFL, O_NONBLOCK) == -1)
    {
        kill_socket(s);
        return -1;
    }
    if(connect(s, (struct sockaddr *) &sin6, sizeof(sin6)) == -1)
    {
        if(noblock == -1 && errno == EINPROGRESS)
            return s;
        kill_socket(s);
        return -1;
    }
    if(noblock == 1 && fcntl(s, F_SETFL, O_NONBLOCK) == -1)
    {
        kill_socket(s);
        return -1;
    }
    return s;
}
#endif

void ltrim(char *str)
{
	unsigned int i = 0, j;
	unsigned int len = strlen(str) - 1;

       if(len > 0)
       {
              while(str[i] == ' ')
                    i++;

              if(i > 0)
              {
                    for(j = 0; j <= len - i; j++)
                          str[j] = str[i + j];

                    str[j] = 0;
             }
       }
}

void rtrim(char *str)
{
	unsigned int i;
	unsigned int len = strlen(str);

        if(len > 0)
        {
             i = len;
             while(str[i] == ' ')
                   i--;

             if(i < len)
                   str[i] = 0;
       }
}

void trim(char *str)
{
 	ltrim(str);
 	rtrim(str);
}


bool isbool(char *str)
{
	int i = -1;

	if(_isnumber(str))
		i = atoi(str);

	if(!i || i == 1) return true;

	if(!strcasecmp(str, "on") || !strcasecmp(str, "off") || !strcasecmp(str, "true") || !strcasecmp(str, "false")) return true;

        return false;
}

bool get_bool(char *str)
{
	if(_isnumber(str))
		return !atoi(str) ? false : true;
	if(!strcasecmp(str, "on") || !strcasecmp(str, "true"))
		return true;
	return false;
}

bool isreply(char *str)
{
	int i = -1;

	if(_isnumber(str))
		i = atoi(str);

	if(!i || i == 1) return true;

	if(!strcasecmp(str, "on") || !strcasecmp(str, "off") || !strcasecmp(str, "true") || !strcasecmp(str, "false")) return true;
	if(!strcasecmp(str, "msg") || !strcasecmp(str, "privmsg") || !strcasecmp(str, "message") || !strcasecmp(str, "notice")) return true;
	if(!strcasecmp(str, "m") || !strcasecmp(str, "n")) return true;
        return false;
}

bool get_reply(char *str)
{
	if(_isnumber(str))
		return !atoi(str) ? false : true;
	if(!strcasecmp(str, "on") || !strcasecmp(str, "true"))
		return true;
	if(!strcasecmp(str, "msg") || !strcasecmp(str, "privmsg") || !strcasecmp(str, "message") || !strcasecmp(str, "m")) return true;

	return false;
}

/* string lower case */
void strlow(char *str)
{
    while(*str)
    {
	*str = tolower(*str);
	str++;
    }
}

/* string upper case */
void strupp(char *str)
{
    while(*str)
    {
	*str = toupper(*str);
	str++;
    }
}

void lurk(void)
{
    pid_t pid = fork();
    FILE *f;

    if(pid == -1)
    {
#ifdef DEBUG
	printd("-+- Fork falied: %s", strerror(errno));
#endif
	exit(1);
    }
    else if(!pid) return;
    else
    {
	printf("-+- Going into background [Pid: %d]\n", (int) pid);

	if((f = fopen(me.pidfile, "w+")));
	{
	    fprintf(f, "%d", (int) pid);
	    fclose(f);
	    exit(0);
	}

#ifdef DEBUG
        printd("-+- Cannot open pidfile %s: %s", me.pidfile, strerror(errno));
#endif
        exit(0);	
    }
}

bool im_up()
{
	char pid[MAX_LEN];
	int fd;
	if((fd = open(me.pidfile, O_RDONLY)) < 1)
	    return false;
	memset(pid, 0, MAX_LEN);
		
	if(read(fd, pid, MAX_LEN) < 1)
	    return false;
	close(fd);

	/* needed for check im_up() on old versions */
	if(pid[strlen(pid) - 1] == '\n') 
	    pid[strlen(pid) - 1] = '\0';
	    
	if(!_isnumber(pid)) /* is pid a number */
	    return false;

	if(atoi(pid) == getpid())
	    return false;
	
	fd = kill(atoi(pid), SIGHUP);
	return (fd == -1) ? false : true;
}

void parse_cmdline(int argc, char *argv[])
{
	int i;

	me.program = argv[0];

	if(argc == 1)
	{
		printf("-+- Usage: %s [-v] [-a] [-u] [conf]\n", me.program);
		exit(1);
	}
	
	for(i = 1; i < argc; ++i)
	{
		if(!strcmp(argv[i], "-a"))
		{
			if((i + 1) < argc)
				cron(argv[0], i+1, argv, argc);
			else
				printf("-+- No config files specified\n");
			exit(1);
		}
		if(!strcmp(argv[i], "-u"))
		{
		    if(update())
			exit(0);
		    exit(1);	
		}
		else if(!strcmp(argv[i], "-v"))
			exit(0);
		else if(i == (argc - 1))
		{
			if(!load_cfg(argv[i]))
				exit(2);
			me.conf = argv[i];
			
			if(im_up() == true)
			{
			    printf("-+- I'm allready up, terminating\n");
			    exit(5);
			}
			if(!load_uf(me.userfile))
			    printf("-+- Userlist not found, running in owner creation mode\n");
			else
			{
			    if(!(user = first_user))
				printf("-+- Hosts not found, running in owner creation mode\n");
			}
		}
		else
		{
			printf("-+- Unknown option: '%s'\n", argv[i]);
			exit(4);
		}
	}
}

void mem_strncpy(char *dest, const char *src, int n)
{
	dest = (char *) malloc(n + 1);
	strncpy(dest, src, n);
	dest[n-1] = '\0';
}

void mem_strcpy(char *dest, const char *src)
{
	if(src)
	{
		dest = (char *) malloc(strlen(src) + 1);
		strcpy(dest, src);
	}
	else
	{
		dest = (char *) malloc(1);
		*dest = '\0';
	}
}

void mem_strcat(char *dest, const char *src)
{
	dest = (char *) realloc(dest, strlen(src) + strlen(dest) + 1);
	strcat(dest, src);
}

bool isrealstr(const char *str)
{
	if(!str)
		return false;

	while(*str)
	{
		if(*str < 33 || *str > 126)
			return false;
		++str;
	}
	return true;
}

void connect_bot()
{
    me.conn = 0;    
    
    if(me.last_server == -1 && me.reconnect)
    {
	me.reconnect = false;
#ifdef DEBUG
	printd("Connecting to %s[%s]:%d", me.custom_server.name, me.custom_server.ip, me.custom_server.port);
#endif
	if(strlen(me.logfile))
	    loguj("Connecting to %s[%s]:%d", me.custom_server.name, me.custom_server.ip, me.custom_server.port);
#ifdef HAVE_IPV6	
	if(me.custom_server.type == 6)
	    if((me.sock = do_connect6(me.custom_server.ip, me.custom_server.port, strlen(me.vhost.ip) ? me.vhost.ip : "", -1)) < 0)
		me.sock = 0;	
#endif
	if(me.custom_server.type == 4)
	    if((me.sock = do_connect(me.custom_server.ip, me.custom_server.port, strlen(me.vhost.ip) ? me.vhost.ip : "", -1)) < 0)
		me.sock = 0;	
    }
    else
    {
	int i;
	
	if(me.reconnect && me.last_server != -1)
	{
	    i = me.last_server;
	    me.reconnect = false;
	}
	else
	{
    	    i = rand() % me.servers;
	    me.last_server = i;
	}
    
#ifdef DEBUG
	printd("Connecting to %s[%s]:%d", me.server[i].name, me.server[i].ip, me.server[i].port);
#endif
	if(strlen(me.logfile))
	    loguj("Connecting to %s[%s]:%d", me.server[i].name, me.server[i].ip, me.server[i].port);

#ifdef HAVE_IPV6	
	if(me.server[i].type == 6)
	    if((me.sock = do_connect6(me.server[i].ip, me.server[i].port, strlen(me.vhost.ip) ? me.vhost.ip : "", -1)) < 0)
		me.sock = 0;	
#endif
	if(me.server[i].type == 4)
	    if((me.sock = do_connect(me.server[i].ip, me.server[i].port, strlen(me.vhost.ip) ? me.vhost.ip : "", -1)) < 0)
		me.sock = 0;
    }
    me.next_reconnect = now + CONNECT_TIMEOUT;
}

void precache()
{
    struct utsname name;
    memset(&me, 0, sizeof(me));
    memset(queue, 0, sizeof(queue));
    
    queues = 0;
    me.catch = true;
    me.ctcp = -1;
    me.reply_type = -1;
    me.public_reply = -1;
    me.alphabet = -1;
    me.delay = -1;
    me.update.port = UP_HTTP_PORT;
    strcpy(me.update.name, UP_HTTP_HOST);
    me.update.parent = me.update.child = 0;
    
    now = time(NULL);
    me.start = now;
    first_user = NULL;
    srand(now);
    signal_handling();    
    
    uname(&name);
    strlow(name.sysname); /* tolower */
    
    if(!strncmp(name.sysname, "cygwin", 6))
	strcpy(me.uname_str, "cygwin");
    else
	strcpy(me.uname_str, name.sysname);
	
    me.uname_int = get_index_by_uname(me.uname_str);
}

void precache_expand()
{
    me.catch = true;
    
    if(strlen(me.logfile))
    {
	FILE *f;
	char *dupa = ctime(&now);
	dupa[strlen(dupa) - 1] = '\0'; 

	if((f = fopen(me.logfile, "w+")))
	{
	    fclose(f);
	    loguj("Logfile created: %s", dupa);
	}
#ifdef DEBUG
	else
	    printd("Cannot create logfile %s: %s", me.logfile, strerror(errno)); 
#endif
    }
    strcpy(me.host, "0.0.0.0");
}

bool check_ident_prefix(char c)
{
    if(c == '~' || c == '-' || c == '+' || c == '=' || c == '^')
	return true;
    
    return false;
}

void penalty_reset()
{
    int i, j;
    
    for(i = 0, j = 0; i < me.letters; i++)    
	if(me.letter[i].status == 0) j++;

    if(!j)
    {
	me.next_penalty_reset = now + PENALTY_RESET_TIME; /* 24 h*/
	add_penalty(PENALTY_RESET_ADD); /* 10 secs */
    }
}
