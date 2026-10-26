#ifndef KNB_SSL_H
#define KNB_SSL_H

#ifdef HAVE_SSL

int init_ssl(void);
void cleanup_ssl(void);
int ssl_attach(int fd);
int ssl_handshake(void);
int ssl_read_sock(char *buf, int len);
int ssl_write_sock(const char *buf, int len);
void ssl_free(void);
int ssl_want_io(void);

#endif /* HAVE_SSL */

int b64_encode(const unsigned char *in, int inlen, char *out, int outmax);

#endif
