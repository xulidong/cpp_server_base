#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#define MAX_SIZE 1024
#define ERR_EXIT(msg) do { \
    printf("errno:%d\n", errno); \
    perror(msg); \
    exit(EXIT_FAILURE); \
}while(0)

struct msgbuf {
    long mtype;
    char mtext[MAX_SIZE];
};

int main() {
    key_t key = ftok(".", 'a');
    int msgid = msgget(key, 0);
    if (-1 == msgid) {
        ERR_EXIT("msgget error"); 
    }

    int pid = getpid();
    size_t pidlen = sizeof(pid);

    struct msgbuf msg;
    memset(&msg, 0, sizeof(msg));
    msg.mtype = 1;// 消息类型定义为1
    *((int*)msg.mtext) = pid;// 消息的前几个字节放pid
    
    while(fgets(msg.mtext + pidlen, MAX_SIZE, stdin) != NULL) {
        printf("---------\n");
        if (msgsnd(msgid, &msg, pidlen + strlen(msg.mtext + pidlen), 0) < 0) {
            ERR_EXIT("msgsnd error"); 
        } 
        printf("=========\n");
        // 前几个字节是自己的pid不需要清空
        memset(msg.mtext + pidlen, 0, MAX_SIZE - pidlen);
        // 接收消息类型为自己pid的消息
        if (msgrcv(msgid, &msg, MAX_SIZE, pid, 0) < 0) {
            ERR_EXIT("msgrcv error"); 
        }
        fputs(msg.mtext + pidlen, stdout);
        memset(msg.mtext + pidlen, 0, MAX_SIZE - pidlen);
        printf("xxxxxxxxx\n");
    }

    return 0;
}
