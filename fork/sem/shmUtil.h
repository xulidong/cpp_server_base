#ifndef __SHM_UTIL__
#define __SHM_UTIL__

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define ERR_PRINT(msg) do { \
    printf("errno: %d\n", errno); \
    perror(msg); \
} while(0)

#define SHM_OK 0
#define SHM_ERROR_BASE 3000
#define SHM_ERROR_PARAM SHM_ERROR_BASE + 1 

int shmflag = 0;
int shmkey;

/*
 * 创建共享内存
 * */
int shm_create(char* shmseedfile, int shmsize, int *pshmid) {
    if (NULL == shmseedfile || NULL == pshmid) {
       return SHM_ERROR_PARAM; 
    }

    if (shmflag == 0) {
        shmkey = ftok(shmseedfile, 'c'); 
        if (-1== shmkey) {
            ERR_PRINT("shm_create ftok error"); 
            return -1;
        }
        shmflag = 1;
    } 

    *pshmid = shmget(shmkey, shmsize, IPC_CREAT | 0666);
    if (-1 == *pshmid) {
        ERR_PRINT("shm_create shmget error"); 
        return -1;
    }
    return SHM_OK;
}

/*
 * 关联共享内存
 * */
int shm_map(int shmid, void **mapaddr) {
    void *ptmp = (void *) shmat(shmid, 0, SHM_RND);
    if (-1 == (int)ptmp) {
        ERR_PRINT("shm_map shmat error"); 
       return -1; 
    }
    *mapaddr = ptmp;
    return SHM_OK;
}

/*
 * 取消关联内存
 * */
int shm_unmap(void *unmapaddr) {
    int ret = shmdt((char*)unmapaddr);
    if (-1 == ret) {
        ERR_PRINT("shm_map shmdt error"); 
        return -1; 
    }
    return SHM_OK;
}

int shm_delete(int shmid) {
    int ret = shmctl(shmid, IPC_RMID, NULL);   
    if (-1 == ret) {
        ERR_PRINT("shm_delete shmctl error"); 
        return -1;
    }
    return SHM_OK;
}

#endif
