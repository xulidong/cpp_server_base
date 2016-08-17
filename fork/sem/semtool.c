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
    int semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);
    if (-1 == semid) {
        if (EEXIST == errno) {
            printf("exist"); 
        }
        ERR_PRINT("semget error");
    }
    return semid; 
}

int sem_open(key_t key) {
    int semid = semget(key, 0, 0);
    if (-1 == semid) {
        ERR_PRINT("semget error");
    }
    return semid; 
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

int sem_getval(int semid) {
    // 因为信号量可以创建多个，第二个参数是要操作信号量的下标
    // GETVAL 最后一个参数被忽略
    int ret = semctl(semid, 0, GETVAL, 0);
    if (-1 == ret ) {
        ERR_PRINT("getval error");
        return ret;
    }
	printf("current val is %d\n", ret);
    return ret;
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

int sem_getmode(int semid) {
    union semun su;
    struct semid_ds sem;
    su.buf = &sem;
    int ret = semctl(semid, 0, IPC_STAT, su);
    if (ret == -1) {
        ERR_PRINT("semctl error");
    }

    printf("current permissions is %o\n", su.buf->sem_perm.mode);
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
    }

    printf("current permissions is %o\n", su.buf->sem_perm.mode);
    sscanf(mode, "%o", (unsigned int *)&su.buf->sem_perm.mode);
    ret = semctl(semid, 0, IPC_SET, su);
    if (ret == -1) {
        ERR_PRINT("semctl error");
	}
	printf("update permissions to %o\n", su.buf->sem_perm.mode);

    return ret;
}


void usage() {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "semtool -c 创建信号量\n");
    fprintf(stderr, "semtool -d 删除信号量\n");
    fprintf(stderr, "semtool -p 信号量p操作\n");
    fprintf(stderr, "semtool -v 信号量v操作\n");
    fprintf(stderr, "semtool -s <val> 设置信号量初始值\n");
    fprintf(stderr, "semtool -g 获取信号量值\n");
    fprintf(stderr, "semtool -f 打开权限\n");
    fprintf(stderr, "semtool -m <mode> 设置信号量权限\n");
}

int main(int argc, char* argv[]) {
	// 第一个值放在opt中
	// 第二个值放在全局变量optarg中
    int opt = getopt(argc, argv, "cdpvs:fgm:");
    if ('?' == opt) {
        return 0;
    }
    if (-1 == opt) {
       usage();
       exit(EXIT_FAILURE);
    }

    key_t key = ftok(".", 'a');
    int semid = -1;
	switch (opt) {
    case 'c':
        sem_create(key);
        break;
    case 'd':
        semid = sem_open(key);
        sem_d(semid);
        break;
    case 'f':
        semid = sem_open(key);
        sem_getmode(semid);
        break;
    case 'g':
        semid = sem_open(key);
        sem_getval(semid);
        break;
    case 'm':
        semid = sem_open(key);
        sem_setmode(semid, argv[2]);
        break;
    case 'p':
        semid = sem_open(key);
        sem_p(semid);
        sem_getval(semid);
        break;
    case 's':
        semid = sem_open(key);
        sem_setval(semid, atoi(optarg));
        break;
    case 'v':
        semid = sem_open(key);
        sem_v(semid);
        sem_getval(semid);
		break;
	default:
		usage();
		break;
	}

    return 0;
}
