#include <stdio.h>
#include <cstring>
#define PG "8.4.5"
int main() {
  char dbname[100];
  memset(dbname, 0, 100);
  /* strcpy(dbname, "dbname=");
  strcat(dbname, "postgres");
  strcat(dbname, ";"); 
  char username[] = "dbuser=yanghao;";
  printf("dbname:%s\n", dbname);
  printf("username:%s\n", username);
  char port[] = "port=1234;";
  int portlen = strlen(port);
  int dbnamelen = strlen(dbname);
  int userlen = strlen(username);
  int connlen = sizeof(int) + dbnamelen + userlen;
	char connstr[connlen + 1];
  memset(connstr, 0, connlen + 1);
  strncat(connstr, port, portlen);
  strncat(connstr, dbname, dbnamelen);
  strncat(connstr, username, userlen);
  */
  char name[] = "postgres";
  char username[] = "yanghao";
  int len = 100;
  sprintf(dbname, "%d%s%s", len, "CSOT", "LOAD");
  printf("str:%s\n", dbname);
  char test[20];
  memset(test, 0, 20);
  for (int i = 0; i < 20; ++i) {
    test[i] = ' ';
  }
  printf("len:%d,str:%s\n", strlen(test), test);
  char pg[50];
  memcpy(pg, PG, strlen(PG));
  printf("pg:%s\n", pg);
}
