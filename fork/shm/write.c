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
    printf("errno: %d\n", errno);\
    perror(msg);\
    exit(EXIT_FAILURE);\
} while(0)

/*
 * 1 共享内存别的程序占用，删除共享内存不会立即删除
 * 2 此时key将变成0，即其他进程不能再通过key来获取shmid
 * 3 只有引用共享内存的所有进程都退出，系统才会删除共享内存
 * */

struct Teacher {
    char name[10];
    int age;
};

int main() {
    key_t key = 0x1234;
    // 创建一片共享内存，大小为sizeof(struct Teacher)
    int shmid = shmget(key, sizeof(struct Teacher), 0666 | IPC_CREAT);
    if (-1 == shmid) {
        if (ENOENT == errno) {
            printf("not exist\n");
        }
        if (EEXIST == errno) {
            printf("exist"); 
        }
        ERR_EXIT("shmget error");
    }

    struct Teacher tch = {"xulidong", 26};
    // 获取共享内存的首地址，可以用ipcs查看，nattch就是引用次数
    void* pshm = shmat(shmid, NULL, 0);
    memcpy(pshm, &tch, sizeof(tch)); 
    shmdt(pshm);// 共享内存的引用技术减1

    return 0;
}
