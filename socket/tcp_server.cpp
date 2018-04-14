#include <cstdio>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>

#define MAXRECVLEN 10000
#define PORT 9999
#define ADDRESS "127.0.0.1"
#define LISTENQUEUELEN 10

using namespace std;

SSL_CTX* InitServerCTX(void) {
  SSL_CTX *ctx = NULL;
  #if OPENSSL_VERSION_NUMBER >= 0x10000000L
  const SSL_METHOD *method;
  #else
  SSL_METHOD *method;
  #endif
  SSL_library_init();
  OpenSSL_add_all_algorithms(); /* load & register all cryptos, etc. */
  SSL_load_error_strings();  /* load all error messages */
  method = SSLv23_client_method();/* create new server-method instance */
  ctx = SSL_CTX_new(method);  /* create new context from method */
  if( ctx == NULL )
  {
    printf("ssl ctx failed!");
  }
  return ctx;
}

void LoadCertificates(SSL_CTX* ctx,char* CertFile,char* KeyFile) {
  //New lines
  SSL_CTX_load_verify_locations(ctx, CertFile, KeyFile);
  SSL_CTX_set_default_verify_paths(ctx);
  //End new lines
  /* set the local certificate from CertFile */
  SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM);
  /* set the private key from KeyFile (may be the same as CertFile) */
  SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM);

  /* verify private key */
  if (!SSL_CTX_check_private_key(ctx)) {
    fprintf(stderr,"Private key does not match the public certificate\n");
    return;
  }
  printf("LoadCertificates Compleate Successfully.....\n");
}

void ShowCerts(SSL* ssl) {
  X509 *cert;
  char *line;
  cert = SSL_get_peer_certificate(ssl);/* Get certificates (if available) */
  if (cert != NULL) {
    printf("Server certificates:\n");
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    printf("Subject: %s\n", line);
    free(line);
    line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    printf("Issuer: %s\n", line);
    free(line);
    X509_free(cert);
  }
  else {
    printf("No certificates.\n");
  }
}

int main() {
  SSL_CTX *ctx;
  SSL_library_init();
  ctx = InitServerCTX();
  
  char buf[MAXRECVLEN];
  int listenfd, connectfd;
  struct sockaddr_in server;
  struct sockaddr_in client;
  socklen_t addrlen;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
     std::cout << "socket() error. Failed to initiate a socket" << std::endl;
     exit(1);
  }

  /* set socket option */
  int opt = SO_REUSEADDR;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  inet_pton(AF_INET, ADDRESS, &server.sin_addr.s_addr);
  if (bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
    std::cout << "bind error." << std::endl;
    exit(1);
  }
  if (listen(listenfd, LISTENQUEUELEN) == -1) {
    std::cout << "listen error." << std::endl;
    exit(1);
  }
  addrlen = sizeof(client);
  SSL *ssl;
  while (true) {
    if ((connectfd = accept(listenfd, (struct sockaddr *)&client, &addrlen)) == -1) {
      std::cout << "accept error." << std::endl;
      exit(1);
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("You got a connection from client's ip %s, port %d at time %ld.%ld\n",
      inet_ntoa(client.sin_addr), htons(client.sin_port), tv.tv_sec, tv.tv_usec);
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, connectfd);
    SSL_accept(ssl);
    //ShowCerts(ssl);
    int bytes = -1;
    while (1) {
      memset(buf, 0, MAXRECVLEN);
      bytes = SSL_read(ssl, buf, MAXRECVLEN);
    if (bytes > 0) {
      buf[bytes] = 0;
      printf("Client msg:%s\n", buf);
    } else {
      std::cout << "recv error." << std::endl;
      close(connectfd);
      SSL_free(ssl);
      break;
    }
    SSL_write(ssl, buf , strlen(buf));
    }
  }
  close(listenfd);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  return 0;
}
