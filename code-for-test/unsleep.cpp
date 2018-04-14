#include <sys/time.h>
#include <time.h>
#include <string>
#include <iostream>

#include <sys/epoll.h>

void usleep(uint64_t usec) {
  struct timeval tv;
  tv.tv_sec = usec / 1000000;
  tv.tv_usec = usec % 1000000;
  int epfd = epoll_create(1);
  epoll_wait(epfd, NULL, 1, &tv);
  // select(0, nullptr, nullptr, nullptr, &tv);
}
/*
void usleep2(uint64_t usec) {
  struct epoll_event;
  int epfd;
  struct timeval tv;
  epfd=epoll_create(1);
  int nfds = epoll_wait(epfd, NULL, 1, 1000);
  gettimeofday(&tv , NULL);
}
*/
void print() {
  int count = 10;
  while (count > 0) {
    std::cout << "time is arrive." << std::endl;
    usleep(5000000);
    count--;
  }
}

int main() {
  print();
}
