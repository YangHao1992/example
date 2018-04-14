 #include <stdio.h>
 #include <sys/time.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <stdlib.h>
 #include <string.h>

void sampleKernelVersion() {
  int f=0;
  char buffer[80]="";
  char *file="/proc/sys/kernel/version";
  f = open(file, O_RDONLY);
  if (f == 0)
  {
      printf("error to open: %s\n", file);
      exit(EXIT_FAILURE);
  }
  read(f, (void *)buffer, 80);
  buffer[strlen(buffer)-1]=0;                 /* 简单实现tr()函数的功能 */
  printf("当前内核版本：\t%s\n", buffer);
  close(f);
}

int main() {
  sampleKernelVersion();
}
