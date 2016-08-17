#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "sockSrv.h"
#include "semUtil.h"
#include "shmUtil.h"

int g_key = 0x333;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

int initSrv() {
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
    return 0;
}


void test_sem() {
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

void test_mutex() {
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

void *thread_routine(void* arg) {
    if (NULL == arg) {
        return NULL;
    }

    pthread_detach(pthread_self());

    int ret = 0;
    int connfd = *(int*)arg;
    free(arg);

    int waitseconds = 5;
    unsigned char recvbuf[1024];
    int len = sizeof(recvbuf);
    while(1) {
        memset(recvbuf, 0, sizeof(recvbuf));
        ret = sockSrvRecv(connfd, recvbuf, &len, waitseconds);
        if (ret != 0) {
            if (SOCK_ERROR_TIMEOUT == ret) {
                continue; 
            }
            printf("sockSrvRecv error:%d\n", ret); 
            break;
        }
        
        test_sem();
        printf("recvbuf: %s\n", recvbuf);

        ret = sockSrvSend(connfd, recvbuf, len, waitseconds);
        if (ret != 0) {
            printf("sockSrvSend error:%d\n", ret); 
            break;
        }
    }
    close(connfd);
    return NULL;
}

int main() {
    signal(SIGPIPE, SIG_IGN);

    initSrv();

    int ret = 0;
    int listenfd = 0;
    ret = sockSrvInit(8001, &listenfd); 
    if (ret != 0) {
        printf("sockSrvInit error:%d\n", ret);
        return -1;
    }

    int waitseconds = 30;
    while(1) {
        int connfd = 0;
        ret = sockSrvAccept(listenfd, &connfd, waitseconds); 
        if (ret == SOCK_ERROR_TIMEOUT) {
            printf("sockSrvAccept timeout\n");
            continue;
        }
            
        int *pconn = (int*)malloc(sizeof(int));
        *pconn = connfd;
        pthread_t tid;
        pthread_create(&tid, NULL, thread_routine, (void*)pconn);
    }

    return 0;
}
