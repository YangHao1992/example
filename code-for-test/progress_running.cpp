#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <iostream>
#define BUFSZ PIPE_BUF 

int main(int argc, char *argv[]) { 
	FILE* fp; 
	int count; 
	char buf[BUFSZ]; 
	char command[150]; 
	sprintf(command, "ps -ef | grep postgres | grep 27575 | grep con | grep -v grep | wc -l"); 

	if ((fp = popen(command,"r")) == NULL) {
		std::cout << "popen error" << std::endl;
	}

	if ((fgets(buf,BUFSZ,fp)) != NULL) {
		count = atoi(buf); 
		if (count == 0) 
			std::cout << "postgres 27575 not found!" << std::endl; 
		else if (count == 1)
			std::cout << "postgres 27575 exist!" << std::endl;
		else
			std::cout << "ps result:" << count << std::endl;
	}
	pclose(fp);
}
