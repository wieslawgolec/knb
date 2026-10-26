void propaganda();
void error(char *msg, ...);
bool copy(char *from, char *to);
bool chan_prefix(char c);
bool cmd_prefix(char c);
void cron(char *prog, int i, char *argv[], int argc);
void str2words(char *word, char *str, int x, int y);
void parse_cmdline(int argc, char *argv[]);
#ifdef DEBUG
void printd(char *format, ...);
#endif
void loguj(char *format, ...);
void sock_write(int fd, char *format, ...);
int count(char *arr[]);
bool _isnumber(char *str);
void kill_socket(int fd);
int do_connect(char *server, int port, char *vhost, int noblock);
#ifdef HAVE_IPV6
int do_connect6(char *server, int port, char *vhost, int noblock);
#endif
bool get_bool(char *str);
bool isbool(char *str);
void ltrim(char *str);
void rtrim(char *str);
void trim(char *str);
void strlow(char *str);
void strupp(char *str);
void lurk();
bool im_up();
void mem_strncpy(char *dest, const char *src, int len);
void mem_strcpy(char *dest, const char *src);
void mem_strcat(char *dest, const char *src);
bool isrealstr(const char *str);
void connect_bot();
void precache();
void precache_expand();
bool check_ident_prefix(char c);
void penalty_reset();
bool get_reply(char *str);
bool isreply(char *str);
