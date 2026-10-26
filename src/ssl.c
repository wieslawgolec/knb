/*
 * Optional OpenSSL client support + small base64 helper for SASL PLAIN.
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

/* Base64 encode for SASL PLAIN (no external deps) */
int b64_encode(const unsigned char *in, int inlen, char *out, int outmax)
{
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, o = 0;

    for (i = 0; i + 2 < inlen; i += 3) {
        if (o + 4 >= outmax)
            return -1;
        out[o++] = tbl[(in[i] >> 2) & 0x3f];
        out[o++] = tbl[((in[i] & 0x3) << 4) | ((in[i + 1] >> 4) & 0xf)];
        out[o++] = tbl[((in[i + 1] & 0xf) << 2) | ((in[i + 2] >> 6) & 0x3)];
        out[o++] = tbl[in[i + 2] & 0x3f];
    }
    if (i < inlen) {
        if (o + 4 >= outmax)
            return -1;
        out[o++] = tbl[(in[i] >> 2) & 0x3f];
        if (i + 1 < inlen) {
            out[o++] = tbl[((in[i] & 0x3) << 4) | ((in[i + 1] >> 4) & 0xf)];
            out[o++] = tbl[((in[i + 1] & 0xf) << 2)];
            out[o++] = '=';
        } else {
            out[o++] = tbl[((in[i] & 0x3) << 4)];
            out[o++] = '=';
            out[o++] = '=';
        }
    }
    if (o >= outmax)
        return -1;
    out[o] = '\0';
    return o;
}
