#include <cstdio>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#define MAXRECVLEN 10000
#define PORT 9999
#define ADDRESS "127.0.0.1"
#define LISTENQUEUELEN 10

using namespace std;
int main() {
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
  while (true) {
    if ((connectfd = accept(listenfd, (struct sockaddr *)&client, &addrlen)) == -1) {
      std::cout << "accept error." << std::endl;
      exit(1);
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("You got a connection from client's ip %s, port %d at time %ld.%ld\n",
      inet_ntoa(client.sin_addr), htons(client.sin_port), tv.tv_sec, tv.tv_usec);
    int iret = -1;
    while (true) {
      memset(buf, 0, MAXRECVLEN);
      iret = recv(connectfd, buf, MAXRECVLEN, 0);
      if (iret > 0) {
        printf("%s\n", buf);
      } else {
        std::cout << "recv error." << std::endl;
        close(connectfd);
        break;
      }
      send(connectfd, buf, iret, 0);
    }
  }
  close(listenfd);
  return 0;
}
