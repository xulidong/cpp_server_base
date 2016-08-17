#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(msg) do {\
    printf("errno:%d\n", errno);\
    perror(msg); \
    exit(EXIT_FAILURE); \
} while(0)

struct Teacher {
    char name[10];
    int age;
};

int main() {
    key_t key = 0x1234;
    // 创建一片共享内存，大小为sizeof(struct Teacher)
    int shmid = shmget(key, sizeof(struct Teacher), 0666);
    if (-1 == shmid) {
        if (ENOENT == errno) {
            printf("not exist\n");
        }
        if (EEXIST == errno) {
            printf("exist"); 
        }
        ERR_EXIT("shmget error");
    }
   
    struct Teacher tch;
    void* pshm = shmat(shmid, NULL, 0);
    memcpy(&tch, pshm, sizeof(tch));
    printf("name:%s\n", tch.name);
    printf("age:%d\n", tch.age);
    shmdt(pshm);
    shmctl(shmid, IPC_RMID, NULL);// 释放共享内存

    return 0;
}
