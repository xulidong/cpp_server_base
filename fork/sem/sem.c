#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
 * p操作 抢占资源 resource--
 * v操作 释放资源 resource++
 * */

#define ERR_PRINT(msg) do { \
    printf("errno: %d\n", errno); \
    perror(msg); \
} while(0)

int sem_create(key_t key) {
    // 第二个参数设置信号量初始值，只在创建时有效，其他情况下设置为0
    // 有的系统val不生效，创建的时候默认为0
    int semid = semget(key, 2, 0666 | IPC_CREAT | IPC_EXCL);
    if (-1 == semid) {
        if (EEXIST == errno) {
            printf("sem_create semget error, key exist\n"); 
            return EEXIST;
        }
        ERR_PRINT("sem_create semget error");
    }
    return semid; 
}

int sem_open(key_t key) {
    int semid = semget(key, 2, 0666);
    if (-1 == semid) {
        if (EINVAL == semid) {
            printf("sem_open semget error, key not exist\n"); 
            return EINVAL;
        }
        ERR_PRINT("sem_open semget error");
    }
    return semid; 
}

int sem_setval(int semid, int val) {
    union semun su;
    su.val = val;
    // 因为信号量可以创建多个，第二个参数是要操作信号量的下标
    int ret = semctl(semid, 0, SETVAL, su);
    if (-1 == ret) {
        ERR_PRINT("setval error");
    }
    return ret;
}

int sem_getval(int semid) {
    // 因为信号量可以创建多个，第二个参数是要操作信号量的下标
    // GETVAL时最后一个参数被忽略
    int ret = semctl(semid, 0, GETVAL, 0);
    if (-1 == ret ) {
        ERR_PRINT("getval error");
        return ret;
    }
    return ret;
}


int sem_p(int semid) {
    // 因为信号量可以创建多个，第1个值是要操作信号量的下标
    // 第二个值 p操作-1 v操作 1
    // 第三个值 
    // IPC_NOWAIT 资源为0时进行P操作，此时不会阻塞等待，而是直接返回资源不可用的错误;
    // SEM_UNDO 当退出进程时对信号量资源的操作撤销，即程序崩溃之后释放占用的资源；
    // 0 不关心时设置为即可。
    struct sembuf sb = {0, -1, SEM_UNDO};
    int ret = semop(semid, &sb, 1);// 最后一个参数：信号量个数
    if (-1 == ret ) {
        ERR_PRINT("sem_p error");
        return ret;
    }
    return ret;
}

int sem_v(int semid) {
    struct sembuf sb = {0, 1, SEM_UNDO};
    int ret = semop(semid, &sb, 1);
    if (-1 == ret ) {
        ERR_PRINT("sem_p error");
        return ret;
    }
    return ret;
}

int main() {
    key_t key = 0x111;
    int semid = sem_create(key);
    if (EEXIST == semid) {
        semid = sem_open(key);
    }
    if (-1 == semid) {
        printf("get semid error\n"); 
        return -1;
    }
        
    int val = sem_getval(semid);
    printf("resCnt = %d\n", val); 
    // int resCnt = 2;// 资源总数会被转化为unsigned类型，val为负数时会报错Result too large
    //sem_setval(semid, resCnt);
    printf("resCnt = %d\n", val); 

    fork();
    sem_p(semid);
    // resCnt 等于1时有一个进程因为获取不到资源将被阻塞，每次只打印出一句
    // resCnt 大于时 两个进程同时都能获取到资源，不会被阻塞，打印两条，然后sleep，然后再打印两条
    {
        int i = 0;
        printf("i: %d, pid: %d \n", i++, getpid());
        sleep(3);   
        printf("i: %d, pid: %d \n", i++, getpid());
    }
    sem_v(semid);

    return 0;
}
