#include <stdio.h>
#include <string.h>

// ignore case sensitive and blank
int strcompare(const char *s1) {
  const char *s2 = "setenable_oracle=true;";
  while (1) {
    // ignore space
    unsigned char ch1 = (unsigned char) *s1++;
    if (ch1 == 32) continue;
	  unsigned char ch2 = (unsigned char) *s2++;
    if (ch2 == 32) continue;
    if (ch1 == ch2 || ((ch1 += ('a' - 'A')) == ch2)) {
      if (ch1 != ';')
        continue;
      else
        return 1;
    } else return 0;
  }
}

int main() {
  char *s1 = "enable_oracle = true;";
  int ret = strcompare(s1);
  printf("result:%d\n", ret);
}
