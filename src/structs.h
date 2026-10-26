struct knb_channel
{
	char name[CHAN_LEN + 1];
	char name_f[CHAN_LEN + 1];
	char key[CHAN_LEN + 1];
	unsigned int status;
	bool cfg;
};

struct knb_server
{
	char name[MASK_LEN + 1];
	char ip[MASK_LEN + 1];
	unsigned int port;
	unsigned int type;
};

struct knb_queue
{
	char nick[NICK_LEN + 1];
	bool used;
	unsigned int type;
	unsigned int prio;
	bool send;
};

struct knb_vhost
{
	char name[MASK_LEN + 1];
	char ip[MASK_LEN + 1];
	unsigned int type;
};

struct knb_userlist
{
	char nick[NICK_LEN + 1];
	char ident[IDENT_LEN + 1];
	char host[MASK_LEN + 1];
	struct knb_userlist *next;
};

struct knb_update
{
	int child;
	int parent;
	pid_t pid;
    	char name[MASK_LEN + 1]; /* hostname */
    	char ip[MASK_LEN + 1];
    	unsigned int port;
    	char version_str[CHAN_LEN + 1];
    	long int version_int;
    	off_t size;
    	char target[CHAN_LEN + 1];
	bool restart;
};

struct knb_nicklist
{
    char nick[NICK_LEN + 1];
    unsigned int status; /* 0 = free, 1 = in use, 2 = unavailable, 3 = bad nick */
    time_t time;
    bool temp;
};

struct knb_special
{
	unsigned int count;
	unsigned int max;
	bool check;
};

struct knb_altnick
{
    bool var;
    bool set;
};

struct knb_user
{
	/* settings */
	char nick[NICK_LEN+1];
	char orignick[NICK_LEN+1];
	char uid[UID_LEN+1];

    	int sock;
    	unsigned int conn;
    	char buf[MID_LEN+1];
    	bool catch;
    	bool catched;
    	unsigned int penalty;
    	char realname[REALNAME_LEN+1];
    	char ident[IDENT_LEN+1];
    	char user[IDENT_LEN+1];

    	char host[MASK_LEN+1];
        char nickappend[NICKAPPEND_LEN+1];
    	struct knb_altnick altnick;
	struct knb_altnick noaltnick;
	
	time_t start;
        char *program;
        char *conf;
	
	int ctcp; /* ctcptype */
	char ctcpreply[CTCP_REPLY_LEN+1];
	
	char reason[REASON_LEN+1];

	char uname_str[CHAN_LEN+1];
	unsigned int uname_int;

    	char userfile[CHAN_LEN+1];
    	char logfile[CHAN_LEN+1];
    	char pidfile[CHAN_LEN+1];
    	time_t timeout;
    	time_t connected;

	/* idle time */
	time_t idle;
	
	/* reply type */
        int reply_type; /* default true */
	int public_reply; /* allowed or disallowed  (default false) */

    	/* channels */
    	struct knb_channel channel[MAX_CHANNELS];
    	int channels;
    	time_t next_chan_check;

    	/* servers */
    	struct knb_server server[MAX_SERVERS];
    	unsigned int servers;
    	struct knb_server custom_server;
    	time_t next_reconnect;
	int last_server;
	bool reconnect;

    	/* nicks */
    	struct knb_nicklist letter[MAX_NICKS];
    	unsigned int letters;

	/* vhost */
	struct knb_vhost vhost;

	/* update */
	struct knb_update update;

	/* special *G* */
	struct knb_special special;
    
	/* selfcheck */
	time_t next_notice;
	time_t next_penalty_reset;
	
	/* ison && send reponse check */
	bool ison;
	bool send;
	
#ifdef DEBUG
	bool dontfork;
#endif
	/* alphabet edition */
	int alphabet;
	/* anty flood detectors */
	int delay;
};



