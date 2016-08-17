#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "shmUtil.h"
#include "semUtil.h"

/*
 * 信号量同步线程 
 *
 * 互斥锁和信号量的区别
 * 互斥锁只能同步线程，不能同步进程，由线程库提供
 * 信号量即可以同步线程又可以同步进程，由Linux内核提供
 * */

int g_key = 0x333;

void incSem() {
    int shmid;
    int *addr = NULL;

    int semid;
    sem_open(g_key, &semid);
    sem_p(semid);// 加锁
    {
        shm_create(".", 0, &shmid);
        shm_map(shmid, (void **)&addr);
        *(int*)addr += 1;
        printf("count:%d\n", *(int*)addr); 
        shm_unmap(addr);
        sleep(1);
    } 
    sem_v(semid);// 解锁
}

void sigHandler(int sig) {
    while(waitpid(-1, 0, WNOHANG)); 
}

void* thread_routine(void *arg) {
    incSem();
    return NULL;
} 

int main() {
    signal(SIGCHLD, sigHandler);

    int ret = 0;

    // 创建共享内存
    int shmid;
    ret = shm_create(".", sizeof(int), &shmid); 
    if (ret != 0) {
        printf("main shm_create error\n");
        return ret; 
    }

    // 创建信号量
    int semid;
    ret = sem_create(g_key, &semid);
    if (ret != 0) {
        if (SEM_ERROR_EXIST == ret) {
            ret = sem_open(g_key, &semid);
            if (-1 == ret) {
                printf("main sem_open error\n");
                return ret;
            }
        } else {
            printf("main sem_create error\n");
            return ret;
        }
    }
    int val = 0;
    sem_setval(semid, 2);
    ret = sem_getval(semid, &val);
    if (ret != 0) {
        printf("main sem_getval error\n");
        return ret; 
    }
    printf("sem val = %d\n", val);
    
    // 创建子线程
    pthread_t tids[20];
    int i = 0;
    for (i = 0; i < 3; i++) {
        pthread_create(&tids[i], NULL, thread_routine, NULL);
    }
    for (i = 0; i < 3; i++) {
        pthread_join(tids[i], NULL);
    }

    return 0;
}
