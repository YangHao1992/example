#include <iostream>
#include <cstring>

#define TempMessageLen 500

static char MsgBuffer[1000];

typedef struct
{
	int  len;
	char logo[5];
	char type[5];
	char hawq_version[20];
	char psql_version[10];
	char os_version[40];
	char macaddr[20];
	char reserverd[10];
} load_req;

#define HQ_VERSION "oushu 4.0.0.0"
#define PSQL_VERSION "8.2.1"

static void GetOsVersion(char *os)
{
	FILE *fp = popen("uname -r", "r");
  fgets(os, sizeof(os), fp);
  pclose(fp);
}

static void PackLoadMsg(char *msg)
{
	char connsrc[TempMessageLen];
	sprintf(connsrc, "port=%s;dbname=%s;dbuser=%s;",
					"1234", "database", "postgres");

  int connlen = strlen(connsrc);

	load_req header;
	strcpy(header.logo, "CSOT");
	strcpy(header.type, "LOAD");
  memcpy(header.hawq_version, HQ_VERSION, strlen(HQ_VERSION));
  memcpy(header.psql_version, PSQL_VERSION, strlen(PSQL_VERSION));
	GetOsVersion(header.os_version);
  int len = 0XFFFFFFFFFFFFFFFF;
	header.len = len;
	memcpy(msg, &header, sizeof(header));
	memcpy(msg + sizeof(header), connsrc, connlen);
	msg[sizeof(header) + connlen] = 0;
}

int main() {
  PackLoadMsg(MsgBuffer);
  printf("msg len:%d\n", strlen(MsgBuffer));
  load_req *head = (load_req *)MsgBuffer;
  printf("len:%d,logo:%s,type:%s,hawq:%s,psql:%s,os:%s,mac:%s\n", head->len,
      head->logo, head->type, head->hawq_version, head->psql_version,
      head->os_version);
  char q[200];
  strncpy(q, MsgBuffer + sizeof(head), 200);
  printf("query:%s\n", q);
}
