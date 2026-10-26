/*
 * Optional OpenSSL client support for IRCNet TLS.
 * Base64 for SASL PLAIN lives in auth.c (always linked).
 */

#include "prots.h"

#ifdef HAVE_SSL

#include <openssl/ssl.h>
#include <openssl/err.h>

static SSL_CTX *ssl_ctx = NULL;
static SSL *ssl_conn = NULL;
static int ssl_last_want = 0;

int init_ssl(void)
{
    if (ssl_ctx)
        return 1;

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    if (!ssl_ctx)
        return 0;

    SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
    return 1;
}

void cleanup_ssl(void)
{
    ssl_free();
    if (ssl_ctx) {
        SSL_CTX_free(ssl_ctx);
        ssl_ctx = NULL;
    }
}

void ssl_free(void)
{
    if (ssl_conn) {
        SSL_free(ssl_conn);
        ssl_conn = NULL;
    }
    me.ssl = NULL;
    ssl_last_want = 0;
}

int ssl_attach(int fd)
{
    if (!ssl_ctx || fd <= 0)
        return 0;

    ssl_free();
    ssl_conn = SSL_new(ssl_ctx);
    if (!ssl_conn)
        return 0;

    SSL_set_fd(ssl_conn, fd);
    SSL_set_connect_state(ssl_conn);
    me.ssl = ssl_conn;
    return 1;
}

int ssl_want_io(void)
{
    return ssl_last_want;
}

int ssl_handshake(void)
{
    int ret;

    ssl_last_want = 0;
    if (!ssl_conn)
        return -1;

    ret = SSL_connect(ssl_conn);
    if (ret == 1)
        return 1;

    {
        int err = SSL_get_error(ssl_conn, ret);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            ssl_last_want = 1;
            return 0; /* in progress */
        }
    }
    return -1;
}

int ssl_read_sock(char *buf, int len)
{
    int ret;

    ssl_last_want = 0;
    if (!ssl_conn)
        return -1;

    ret = SSL_read(ssl_conn, buf, len);
    if (ret > 0)
        return ret;

    {
        int err = SSL_get_error(ssl_conn, ret);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            ssl_last_want = 1;
            return 0;
        }
        if (err == SSL_ERROR_ZERO_RETURN)
            return -1;
    }
    return -1;
}

int ssl_write_sock(const char *buf, int len)
{
    int ret;

    ssl_last_want = 0;
    if (!ssl_conn)
        return -1;

    ret = SSL_write(ssl_conn, buf, len);
    if (ret > 0)
        return ret;

    {
        int err = SSL_get_error(ssl_conn, ret);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            ssl_last_want = 1;
            return 0;
        }
    }
    return -1;
}

#endif /* HAVE_SSL */
