#include "prots.h"

void sig_child(int s)
{
    int status;
    while(waitpid(-1, &status, WNOHANG) > 0);
}

void sig_term(int s)
{
    signal(SIGTERM, SIG_IGN);
    send_quit("Got termination signal\r\n");
    if(strlen(me.logfile))
	loguj("Got signal: Termination signal");
    save_uf(me.userfile);
    exit(1);
}

void sig_int(int s)
{
    signal(SIGINT, SIG_IGN);
    send_quit("Terminated by user\r\n");
    if(strlen(me.logfile))
	loguj("Got signal: Terminated by user");
    save_uf(me.userfile);
    exit(1);
}

void sig_hup(int s)
{
    if(strlen(me.logfile))
	loguj("Got signal: Saving userlist");
    save_uf(me.userfile);
    return;
}

void sig_segv(int s)
{
#ifdef DEBUG
    char gdb[MAX_LEN], cmdlist[MASK_LEN], btfile[MAX_LEN];
    FILE *f;
    struct rlimit limit;
#endif
    signal(SIGSEGV, SIG_IGN);
    send_quit("Segmentation fault, please send log to esio(at)hoth.amu.edu.pl");
    if(strlen(me.logfile))
	loguj("Got signal: Segmentation fault, please send log to esio(at)hoth.amu.edu.pl");
    //save_uf(me.userfile);
#ifdef DEBUG    
    snprintf(cmdlist, MASK_LEN, "/tmp/.knb-%d", getpid());
    
    if((f = fopen(cmdlist, "w")))
    {
	fprintf(f, "attach %d\n", getpid());
	fprintf(f, "bt 100\n");
	fprintf(f, "detach\n");
	fprintf(f, "q\n");
	fclose(f);
	
	snprintf(btfile, MASK_LEN, ".gdb-backtrace-%d", getpid());
	snprintf(gdb, MAX_LEN, "gdb -q -x %s > %s 2>&1", cmdlist, btfile);
	system(gdb);
	unlink(cmdlist);
    }
    // enabling core dumps :)
    if(!getrlimit(RLIMIT_CORE, &limit))
    {
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_CORE, &limit);
    }
#endif
    exit(2);
}

void safe_exit(int s)
{
    send_quit("Abnormal program termination");
    if(strlen(me.logfile))
	loguj("Got signal: Abnormal program termination");
    save_uf(me.userfile);
    exit(3);
}

void sig_updated(int s)
{
    char buf[MIN_LEN];
    int ret;
    end_update();
    save_uf(me.userfile);
    send_quit("Restarting");
    ret = unlink(me.pidfile);
    sprintf(buf, "%s %s", me.program, me.conf);
    ret = system(buf);
    exit(0);    
}

void sig_updated2(int s)
{
    end_update();
    save_uf(me.userfile);
}


void signal_handling(void)
{
#define reg_sig(x, y) \
{ \
    memset(&sv, 0, sizeof(sv)); \
    sv.sa_handler = y; \
    sigaction(x, &sv, NULL); \
}

    struct sigaction sv;
    
    reg_sig(SIGPIPE, SIG_IGN);
    reg_sig(SIGCHLD, (sighandler_t) sig_child); 
    reg_sig(SIGTERM, (sighandler_t) sig_term);
    reg_sig(SIGINT, (sighandler_t) sig_int);
    reg_sig(SIGHUP, (sighandler_t) sig_hup);
    reg_sig(SIGUSR1, (sighandler_t) sig_updated);
    reg_sig(SIGUSR2, (sighandler_t) sig_updated2);
    reg_sig(SIGSEGV, (sighandler_t) sig_segv);

#undef reg_sig 
}
