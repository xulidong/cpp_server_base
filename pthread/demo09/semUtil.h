#ifndef __SEM_UTIL__
#define __SEM_UTIL__

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define ERR_PRINT(msg) do { \
    printf("errno: %d\n", errno); \
    perror(msg); \
} while(0)

#define SEM_OK 0
#define SEM_ERROR_BASE 3000
#define SEM_ERROR_PARAM SEM_ERROR_BASE + 1 
#define SEM_ERROR_EXIST SEM_ERROR_BASE + 2 

int sem_create(key_t key, int *psemid) {
    if (NULL == psemid) {
        return SEM_ERROR_PARAM; 
    }

    int semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);
    if (-1 == semid) {
        if (EEXIST == errno) {
            printf("sem_create semget error exist\n"); 
            return SEM_ERROR_EXIST;
        }
        ERR_PRINT("sem_create semget error");
        return -1;
    }
    *psemid =  semid; 
    return SEM_OK;
}

int sem_open(key_t key, int *psemid) {
    if (NULL == psemid) {
        return SEM_ERROR_PARAM; 
    }

    int semid = semget(key, 0, 0);
    if (-1 == semid) {
        ERR_PRINT("sem_open semget error");
        return -1;
    }
    *psemid =  semid; 
    return SEM_OK;
}

int sem_setval(int semid, int val) {
    union semun su;
    su.val = val;
    // 因为信号量可以创建多个，第二个参数是要操作信号量的下标
    int ret = semctl(semid, 0, SETVAL, su);
    if (-1 == ret ) {
        ERR_PRINT("setval error");
    }
    return ret;
}

int sem_getval(int semid, int *pval) {
    // 因为信号量可以创建多个，第二个参数是要操作信号量的下标
    // GETVAL 最后一个参数被忽略
    int ret = semctl(semid, 0, GETVAL, 0);
    if (-1 == ret ) {
        ERR_PRINT("getval error");
        return ret;
    }
	printf("current val is %d\n", ret);
    *pval = ret;
    return SEM_OK;
}

int sem_d(semid) {
    int ret = semctl(semid, 0, IPC_RMID, 0);
    if (ret == -1) {
        ERR_PRINT("semctl error");
    }
    return ret;
}

int sem_p(int semid) {
    // 因为信号量可以创建多个，第1个值是要操作信号量的下标
    // 第二个值 p操作-1 v操作 1
    // 第三个值 
    // IPC_NOWAIT 资源为0时进行P操作，此时不会阻塞等待，而是直接返回资源不可用的错误;
    // SEM_UNDO 当退出进程时对信号量资源的操作撤销；
    // 0 不关心时设置为即可。
    struct sembuf sb = {0, -1, SEM_UNDO};
    int ret = semop(semid, &sb, 1);// 最后一个参数：信号量个数
    if (-1 == ret ) {
        if (EEXIST == errno) {
            printf("exist"); 
        }
        ERR_PRINT("sem_p error");
        return ret;
    }
    return ret;
}

int sem_v(int semid) {
    struct sembuf sb = {0, 1, SEM_UNDO};
    int ret = semop(semid, &sb, 1);
    if (-1 == ret ) {
        if (EEXIST == errno) {
            printf("exist"); 
        }
        ERR_PRINT("sem_p error");
        return ret;
    }
    return ret;
}

int sem_getmode(int semid, mode_t *pmode) {
    union semun su;
    struct semid_ds sem;
    su.buf = &sem;
    int ret = semctl(semid, 0, IPC_STAT, su);
    if (ret == -1) {
        ERR_PRINT("semctl error");
        return ret;
    }

    printf("current permissions is %o\n", su.buf->sem_perm.mode);
    *pmode = su.buf->sem_perm.mode;
    return ret;
}

int sem_setmode(int semid, char *mode)
{
    union semun su;
    struct semid_ds sem;
    su.buf = &sem;

    int ret = semctl(semid, 0, IPC_STAT, su);
    if (ret == -1) {
        ERR_PRINT("semctl");
        return -1;
    }
    printf("current permissions is %o\n", su.buf->sem_perm.mode);
    sscanf(mode, "%o", (unsigned int *)&su.buf->sem_perm.mode);
    ret = semctl(semid, 0, IPC_SET, su);
    if (ret == -1) {
        ERR_PRINT("semctl error");
        return -1;
	}
	printf("update permissions to %o\n", su.buf->sem_perm.mode);

    return ret;
}

#endif
