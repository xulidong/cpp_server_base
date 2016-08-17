#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
 * 在主线程中传递给子线程的地址
 * 在主线中修改了该内存的值
 * 在自线程中的值也将发生改变
 *
 * 解决方案是malloc新的内存
 * 在子线程中释放这个内存
 * */

void *start_routine(void *arg) {
    sleep(1);
    printf("pthread id:%d\n", (int)pthread_self());
    printf("start_routine cnt: %d\n", *(int*)arg); // cnt = 456
    return NULL;
}

int main() {
    int cnt = 123;
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, start_routine, &cnt);
    cnt = 456;
    if (0 != ret) {
        printf("errno:%d", errno);
        perror("main pthread_create error");
        return -1;
    }
    pthread_join(tid, NULL);


    return 0;
}
