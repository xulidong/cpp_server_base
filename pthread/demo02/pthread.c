#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
 * 传递数据到子线程：
 * 1 全局变量
 * 2 传递地址
 * 子进程传递参数给主线程
 * 1 返回地址return 或 pthread_exit()
 * */

typedef struct _Teacher {
    char name[10];
    int age;
} Teacher;

int gNum = 10;

void *start_routine(void *arg) {
    Teacher* ptch = (Teacher *)arg;
    // 访问全局变量
    printf("global: gNum = %d\n", gNum);
    printf("arg:name = %s, age = %d\n", ptch->name, ptch->age++);
    printf("phread id:%d\n", (int)pthread_self()); 
    // 下面两种方式都可以传递arg参数给主线程
    // pthread_exit(arg);
    return arg;
}

int main() {
    Teacher tch = {"xulidong", 26};
    pthread_t tid;
    // 传递参数&tch给子线程
    int ret = pthread_create(&tid, NULL, start_routine, &tch);
    if (0 != ret) {
        printf("errno:%d", errno);
        perror("main pthread_create error");
        return -1;
    }

    Teacher* ptch = NULL;
    pthread_join(tid, (void *)&ptch);
    printf("arg:name = %s, age = %d\n", ptch->name, ptch->age);

    return 0;
}
