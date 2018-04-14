#include <stdio.h>
#include <time.h>
#include <string.h>
int main ()
{
    time_t t;
    struct tm * lt;
    time (&t);//获取Unix时间戳。
    lt = localtime (&t);//转为时间结构。
    char msg[100];
    sprintf(msg, "%d%d%d%d%d%d\n",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果
    printf("len:%d, msg:%s\n", strlen(msg), msg);
    return 0;
}

