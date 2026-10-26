#define KNB_ALPHABET 	"#alphabet"
#define KNB_VERSION_INT	"2000600"
#define KNB_VERSION_STR "0.2.6"
#define KNB_AUTOR	"Esio"
#define KNB_EMAIL	"esio(at)hoth.amu.edu.pl"
#define KNB_SHORT  	"Knb"
#define KNB_FULL 	"Keep nick bot"
#define USERFILE_EXT 	"uf"
#define LOGFILE_EXT	"log"
#define PIDFILE_EXT 	"pid"

#define loop() 		while(1)
#define gotoxy(x,y) 	printf("\e[%i;%if", x, y)
#define clrscr() 	printf("\e[2J \e[1;1f")

#define _NO_LAME_ERRNO

#define MAX_LEN 	4096
#define MID_LEN 	1024
#define MIN_LEN 	512

#define MAX_SERVERS 	20
#define MAX_CHANS 	21
#define MAX_MASKS 	16
#define MAX_QUEUE 	50

#define NO_PRIO		0
#define LOW_PRIO 	1
#define MID_PRIO 	2
#define HIGH_PRIO	3

#define NICK_LEN 	15
#define NICKAPPEND_LEN  40
#define UID_LEN 	9
#define IDENT_LEN 	15
#define REALNAME_LEN 	160

#define MASK_LEN 255

#define CTCP_REPLY_LEN 255

#define REASON_LEN 64

#define CHAN_LEN 	50
#define MAX_CHANNELS 	MAX_CHANS
#define MAX_NICKS 	40

#define CHAN_CHECK_DELAY 60
#define RECONNECT_DELAY 30

#define PENALTY_RESET_TIME 86400 /* 24 h*/
#define PENALTY_RESET_ADD	10 /* 10 secs */
#define TIMEOUT_TIME 	900 /* 15 mins */
#define CONNECT_TIMEOUT 30
#define NICK_UNAVAILABLE 1800 /* 30 mins */

#define SPECIAL_COUNT 	100
#define SPECIAL_RANDOMIZE 50

#ifdef DEBUG
#define DEBUG_FILE 	"debug.log"
#endif

#ifdef SUNOS
#define gethostbyname2(h, p) getipnodebyname(h, p, 0, &errno)
#endif

#ifdef WINDOWS
#ifndef HAVE_IPV6
#define gethostbyname2(h, p) gethostbyname(h)
#endif
#endif

#ifndef INADDR_NONE
#define INADDR_NONE 	(-1)
#endif

#define FLOOD_CMD 1