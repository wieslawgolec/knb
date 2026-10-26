#ifndef KNB_AUTH_H
#define KNB_AUTH_H

/* Called once TCP (and optional TLS) is ready. Sends CAP/NICK/USER. */
void start_irc_register(void);

/* Handle CAP / AUTHENTICATE / SASL numeric replies. Returns true if consumed. */
bool handle_cap_sasl(char **s, int ntok);

#endif
