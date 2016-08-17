#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define CONSUMERS_COUNT 2
#define PRODUCTS_COUNT 2
#define MAX_RES 20 

/*
 * 条件量
 * 生产者消费者问题
 *
 * 条件等待模型
 * 互斥锁和条件等待解决消费者,例如socket链接池
 * pthread_mutex_lock(&g_mutex);
 * while(0 == g_count) {
 *  pthread_cond_wait(&g_cond, &gmutex);
 * }
 * --g_count;
 * pthread_mutex_unlock(&g_mutex);
 * 
 * 条件唤醒模型
 * pthread_mutex_lock(&g_mutex);
 * ++g_count;
 * pthread_cond_signal(&g_cond);
 * pthread_mutex_unlock(&g_mutex);
 * */

pthread_mutex_t g_mutex;
pthread_cond_t g_cond;

pthread_t g_thread[CONSUMERS_COUNT + PRODUCTS_COUNT];
int nready = 0;

void *consume(void* arg) {
    int num = (int) arg;
    while(1) {
        pthread_mutex_lock(&g_mutex);
        {
            while (0 == nready) {
               printf("%d begin wait a condition, count = %d\n", num, nready); 
               /* 
                * 1 丢掉锁
                * 2 线程休眠，进入等待
                * 3 线程等待通知
                * 4 重新获取锁,线程唤醒
                * */
               pthread_cond_wait(&g_cond, &g_mutex);// 让其它线程有机会获取锁
            }
            printf("%d begin consume product, count = %d\n", num, nready);
            --nready;
            printf("%d end consume product, count = %d\n", num, nready);
            pthread_mutex_unlock(&g_mutex);
            sleep(1);
        }
    }
    return NULL;
}

/*
 * 无产品数量上限
 * */
void *produce(void *arg) {
    int num = (int)arg;
    while(1) {
        pthread_mutex_lock(&g_mutex);
        { 
            printf("%d begin procude product, count = %d\n", num, nready);
            ++nready;
            printf("%d end procude product, count = %d\n", num, nready);
            pthread_cond_signal(&g_cond);
            printf("%d signal\n", num);
            pthread_mutex_unlock(&g_mutex);
            sleep(1);
        }
    }
    return NULL;
}

/*
 * 有产品数量上限
 * */
/*
void *produce(void *arg) {
    int num = (int)arg;
    while(1) {
        pthread_mutex_lock(&g_mutex);
        { 
            while(nready > MAX_RES) {
                printf("%d full count: %d\n", num, nready);
                pthread_cond_signal(&g_cond);
                printf("%d signal\n", num);
                pthread_mutex_unlock(&g_mutex);
                sleep(10);
            }
            printf("%d begin procude product, count = %d\n", num, nready);
            ++nready;
            printf("%d end procude product, count = %d\n", num, nready);
            pthread_cond_signal(&g_cond);
            printf("%d signal\n", num);
            pthread_mutex_unlock(&g_mutex);
            sleep(1);
        }
    }
    return NULL;
}
*/

int main() {
    int i = 0;

    // 初始化锁
    pthread_mutex_init(&g_mutex, NULL);
    pthread_cond_init(&g_cond, NULL);

    // 消费线程
    for (i = 0; i < CONSUMERS_COUNT; i++) {
        pthread_create(&g_thread[i], NULL, consume, (void*)i); 
    }

    // 生产线程
    for (i = 0; i < PRODUCTS_COUNT; i++) {
        pthread_create(&g_thread[i], NULL, produce, (void*)i); 
    }
    
    // 线程等待
    for (i = 0; i < CONSUMERS_COUNT + PRODUCTS_COUNT; i++) {
       pthread_join(g_thread[i], NULL); 
    }

    return 0;
}
