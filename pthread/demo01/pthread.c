#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
 * 线程与进程共享数据，可以使用进程中的全局变量
 * 子进程不可以
 *
 * 等待线程退出：pthread_join
 * 线程退出：
 * 1 线程函数之行完成
 * 2 pthread_exit
 * 3 pthread_cancle取消一个执行中的线程
 * 如果在线程中使用exit退出的是整个进程
 *
 * pthread_detach不阻塞主线程，运行之后自动释放，不会产生僵尸线程
 * */

void *start_routine(void *arg) {
    printf("phread id:%d\n", (int)pthread_self()); 
    // pthread_exit(0);
    // pthread_detach(pthread_self());
    return NULL;
}

int main() {
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, start_routine, NULL);
    if (0 != ret) {
        printf("errno:%d", errno);
        perror("main pthread_create error");
        return -1;
    }

    // 如果不sleep，进程很快执行完成之后退出，线程函数执行不到
    // sleep(1);

    // 等待线程退出
    pthread_join(tid, NULL);
    printf("main thread\n");

    return 0;
}
