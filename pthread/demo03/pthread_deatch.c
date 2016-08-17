#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void *start_routine(void *arg) {
    printf("pthread id:%d\n", (int)pthread_self());
    int k = 0;
    for (k = 0; k < *(int*)arg; k++) {
        printf("start_routine loop %d\n", k); 
    }
    return NULL;
}

int main() {
    int ret = 0;
    int i = 0;
    int j = 0;
    int pCnt = 2;
    int lCnt = 3;
    pthread_t tid[10];

    for (i = 0; i < pCnt; i++) {
        ret = pthread_create(&tid[i], NULL, start_routine, &lCnt);
        if (0 != ret) {
            printf("errno:%d", errno);
            perror("main pthread_create error");
            return -1;
        }
        pthread_detach(tid[i]);
    }


    return 0;
}
