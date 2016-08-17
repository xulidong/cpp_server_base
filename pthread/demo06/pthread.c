#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
 * 两种初始化方式一样 
 * */

int g_num = 100;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_routine(void* arg) {
    pthread_mutex_lock(&mutex);
    {
        printf("pthread:%d, g_num: %d\n", (int)pthread_self(), g_num++);
        sleep(1);
    }
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    // pthread_mutex_init(&mutex, NULL);
    int i = 0;
    for (i = 0; i < 3; i++) {
        pthread_t tid;
        pthread_create(&tid, NULL, thread_routine, NULL); 
        pthread_join(tid, NULL);
    }
    
    pthread_mutex_destroy(&mutex);

    return 0;
}
