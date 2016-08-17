#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "shmUtil.h"

int g_key = 0x333;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void incSem() {
    int shmid;
    int *addr = NULL;

    pthread_mutex_lock(&g_mutex);
    {
        shm_create(".", 0, &shmid);
        shm_map(shmid, (void **)&addr);
        *(int*)addr += 1;
        printf("count:%d\n", *(int*)addr); 
        shm_unmap(addr);
        sleep(1);
    } 
    pthread_mutex_unlock(&g_mutex);
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
