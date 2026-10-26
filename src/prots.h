#include <pwd.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/resource.h>

#ifdef SUNOS
#include <errno.h>
#include <sys/uio.h>
#else
#include <sys/errno.h>
#endif

typedef enum { false = 0, true = 1} bool;
typedef void (*sighandler_t)(int);

#include "defines.h"
#include "structs.h"
#include "global-vars.h"
#include "functions.h"
#include "signals.h"
#include "events.h"
#include "update.h"
#include "match.h"
#include "queue.h"
#include "http.h"
#include "cmds.h"
#include "cfg.h"
#include "irc.h"
#include "uf.h"
