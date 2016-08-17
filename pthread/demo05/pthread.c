#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
 * 获取线程的默认属性
 * */
int main() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // 线程分离属性
    int state;
    pthread_attr_getdetachstate(&attr, &state);
    if (PTHREAD_CREATE_JOINABLE == state) {
        printf("joinable\n"); 
    } else if (PTHREAD_CREATE_DETACHED == state) {
        printf("unjoinable\n"); 
    }

    size_t size;
    // 线程栈大小，如果这个值为10M，假如定义int arr[1024 * 1024 * 11];将会出现段错误
    pthread_attr_getstacksize(&attr, &size);
    printf("stack size: %lu\n", size);

    // 线程栈溢出保护区大小
    pthread_attr_getguardsize(&attr, &size);
    printf("guard size: %lu\n", size);
    
    // 线程竞争范围
    int scope;
    pthread_attr_getscope(&attr, &scope);
    if (PTHREAD_SCOPE_PROCESS == scope) {
        printf("scope process\n"); 
    } else if (PTHREAD_SCOPE_SYSTEM == scope) {
        printf("scope system\n"); 
    }

    // 获取线程的调度策略
    int policy;
    pthread_attr_getschedpolicy(&attr, &policy);
    if (SCHED_FIFO == policy) {// 先进先出
       printf("fifo\n"); 
    }else if (SCHED_RR == policy) {// 抢占式
       printf("rr\n"); 
    }else if (SCHED_OTHER == policy) {// 其他 
       printf("other\n"); 
    }

    // 获取设置线程继承的电镀策略
    int inherit;
    pthread_attr_getinheritsched(&attr, &inherit);
    if (PTHREAD_INHERIT_SCHED == inherit) {
        printf("inherit\n"); 
    } else if (PTHREAD_EXPLICIT_SCHED == inherit) {
        printf("explict\n"); 
    }

    // 获取设置线程的调度参数，只需关注调度优先级 
    struct sched_param param;
    pthread_attr_getschedparam(&attr, &param);
    printf("priority: %d\n", param.sched_priority);

    // 设置获取并发级别
    int level = pthread_getconcurrency();
    printf("level: %d\n", level);


    return 0;
}
