#define CFG_NICK 	"nick"
#define CFG_NICKS	"nicks"
#define CFG_CHANNEL	"channel"
#define CFG_SERVER	"server"
#define CFG_SERVER6	"server6"
#define CFG_ALTNICK	"altnick"
#define CFG_NICKAPPEND	"nickappend"
#define CFG_REALNAME	"realname"
#define CFG_IDENT	"ident"
#define CFG_LOGFILE	"logfile"
#define CFG_PIDFILE	"pidfile"
#define CFG_USERFILE	"userfile"
#define CFG_VHOST	"vhost"
#define CFG_CTCP	"ctcptype"
#define CFG_CTCPREPLY   "ctcpreply"
#define CFG_REASON	"reason"

#define CFG_REPLYTYPE	"replytype" /* msg or notice */
#define CFG_PUBLICREPLY	"publicreply" /* allow or dissallow to reply on channel */

#define CFG_NOALTNICK	"noaltnick"

#define CFG_ALPHABET	"alphabet" /* auto add #alphabet */
#define CFG_DELAY	"delay" /* 0 - 10s max - additional penalty for sending ison/nick */

#ifdef DEBUG
#define CFG_DONTFORK	"dontfork"
#endif

bool err_return(int number, char *error, char *var, char *param);
bool set_variable(char *var, char *args, char *error);
bool load_cfg(char *file);

