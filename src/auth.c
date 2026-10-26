#include "prots.h"
#include "auth.h"

void start_irc_register(void)
{
    if (strcasecmp(me.nick, me.orignick))
        strcpy(me.nick, me.orignick);

    me.sasl_state = 0;

    if (strlen(me.sasl_user) && strlen(me.sasl_pass)) {
        me.sasl_state = 1;
        sock_write(me.sock, "CAP LS 302\r\n");
    }

    sock_write(me.sock, "NICK %s\r\n", me.nick);
    sock_write(me.sock, "USER %s 8 * :%s\r\n", me.user, me.realname);
}

static void sasl_send_plain(void)
{
    unsigned char raw[256];
    char b64[400];
    int n = 0;

    /* PLAIN: \0 authorization-id \0 password  (authzid empty -> use authcid) */
    if ((int)strlen(me.sasl_user) + (int)strlen(me.sasl_pass) + 2 >= (int)sizeof(raw))
        return;

    raw[n++] = 0;
    memcpy(raw + n, me.sasl_user, strlen(me.sasl_user));
    n += strlen(me.sasl_user);
    raw[n++] = 0;
    memcpy(raw + n, me.sasl_pass, strlen(me.sasl_pass));
    n += strlen(me.sasl_pass);

    if (b64_encode(raw, n, b64, sizeof(b64)) < 0)
        return;

    sock_write(me.sock, "AUTHENTICATE PLAIN\r\n");
    sock_write(me.sock, "AUTHENTICATE %s\r\n", b64);
}

bool handle_cap_sasl(char **s, int ntok)
{
    int i;

    if (ntok < 3)
        return false;

    /* CAP * LS :... */
    if (!strcmp(s[1], "CAP") && ntok >= 4 && !strcasecmp(s[3], "LS")) {
        int has_sasl = 0;
        for (i = 4; i < ntok && s[i]; i++) {
            if (strstr(s[i], "sasl"))
                has_sasl = 1;
        }
        if (me.sasl_state == 1 && has_sasl && strlen(me.sasl_user)) {
            me.sasl_state = 2;
            sock_write(me.sock, "CAP REQ :sasl\r\n");
        } else {
            me.sasl_state = 0;
            sock_write(me.sock, "CAP END\r\n");
        }
        return true;
    }

    /* CAP * ACK :sasl */
    if (!strcmp(s[1], "CAP") && ntok >= 4 && !strcasecmp(s[3], "ACK")) {
        if (me.sasl_state == 2) {
            me.sasl_state = 3;
            sasl_send_plain();
        }
        return true;
    }

    /* CAP * NAK */
    if (!strcmp(s[1], "CAP") && ntok >= 4 && !strcasecmp(s[3], "NAK")) {
        me.sasl_state = 0;
        sock_write(me.sock, "CAP END\r\n");
        return true;
    }

    /* AUTHENTICATE +  (server ready for payload — already sent in PLAIN path) */
    if (!strcmp(s[0], "AUTHENTICATE") || (ntok >= 2 && !strcmp(s[1], "AUTHENTICATE"))) {
        return true;
    }

    /* 903 RPL_SASLSUCCESS */
    if (!strcmp(s[1], "903")) {
        me.sasl_state = 0;
        sock_write(me.sock, "CAP END\r\n");
#ifdef DEBUG
        printd("SASL authentication successful");
#endif
        if (strlen(me.logfile))
            loguj("SASL authentication successful");
        return true;
    }

    /* 904/905/906/907 failure */
    if (!strcmp(s[1], "904") || !strcmp(s[1], "905") ||
        !strcmp(s[1], "906") || !strcmp(s[1], "907")) {
        me.sasl_state = 0;
        sock_write(me.sock, "CAP END\r\n");
#ifdef DEBUG
        printd("SASL authentication failed (%s)", s[1]);
#endif
        if (strlen(me.logfile))
            loguj("SASL authentication failed (%s)", s[1]);
        return true;
    }

    return false;
}
