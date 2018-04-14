#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

int OpenConnection(){
  int sd;
  struct sockaddr_in addr;
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    printf("create socket failed.\n");
    return 0;
  }
  memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port= htons(9999);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
  if (connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 ) {
    close(sd);
    printf("connect to 127.0.0.1:9999 failed\n");
    return 0;
  }
  return sd;
}

SSL_CTX* InitCTX(void) {
  SSL_CTX *ctx;
  OpenSSL_add_all_algorithms(); /* Load cryptos, et.al. */
  SSL_load_error_strings();  /* Bring in and register error messages */
  ctx = SSL_CTX_new(SSLv23_client_method());  /* Create new context */
  if (ctx == NULL) {
    printf("failed to init ssl ctx.\n");
    return NULL;
  }
  return ctx;
}

void ShowCerts(SSL* ssl) {
  X509 *cert;
  char *line;
  cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
  if (cert != NULL) {
    printf("Server certificates:\n");
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    printf("Subject: %s\n", line);
    free(line);      /* free the malloc'ed string */
    line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    printf("Issuer: %s\n", line);
    free(line);      /* free the malloc'ed string */
    X509_free(cert);    /* free the malloc'ed certificate copy */
  } else {
    printf("No certificates.\n");
  }
}

typedef struct {
  int len;
  char logo[5];
} header;

int main() {
  SSL_CTX *ctx;
  int server;
  SSL *ssl;
  char buf[1024];
  int bytes;
  SSL_library_init();
  ctx = InitCTX();
  server = OpenConnection();
  if (ctx == NULL) {
    printf("ctx is null\n");
  }
  ssl = SSL_new(ctx);     /* create new SSL connection state */
  SSL_set_fd(ssl, server);   /* attach the socket descriptor */
  if (SSL_connect(ssl) == -1) {
    printf("ssl connect failed!\n");
  } else {
    const char *query ="select * from test;";
    header head;
    int sqlLen = strlen(query);
    int msgLen = sqlLen + sizeof(head);
    printf("msg len:%d\n", msgLen);
    memcpy(buf, &head, sizeof(head));
    memcpy(buf + sizeof(head), query, sqlLen);
    printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
    ShowCerts(ssl);       /* get any certs */
    int written_bytes = SSL_write(ssl, buf, strlen(buf));  /* encrypt & send message */
    printf("write bytes:%d\n", written_bytes);
    bytes = SSL_read(ssl, buf, sizeof(buf));/* get reply & decrypt */
    buf[bytes] = 0;
    printf("Received: \"%s\"\n", buf + sizeof(head));
  }
  SSL_free(ssl);       /* release connection state */
  SSL_CTX_free(ctx);       /* release context */
  close(server);        /* close socket */
  return 0;
}
