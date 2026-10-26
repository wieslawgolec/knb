void got_join(char *who, char *ch);
void got_invite(char *from, char *to);
void got_kick(char *who, char *nick, char *from);
void got_quit(char *who, char *why);
void got_ison(char *data);
void got_nick(char *who, char *new_nick);
void got_fail(char *nick, int reason);
void got_ctcp(char *from, char *what);
void got_privmsg(char *from, char *to, char *what);
