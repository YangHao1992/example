#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

int main(int argc, char *argv[]) {
  struct ifreq ifreq;
  int sock = 0;
  char mac[32] = "";

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("error sock");
    return 2;
  }

  strcpy(ifreq.ifr_name, "eth0");
  if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0) {
    perror("error ioctl");
    return 3;
  }

  int i = 0;
  for(i = 0; i < 6; i++){
    sprintf(mac + 3 * i, "%02X:", (unsigned char)ifreq.ifr_hwaddr.sa_data[i]);
  }
  mac[strlen(mac) - 1] = 0;
  printf("MAC: %s\n", mac);

  return 0;
}
