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
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (-1 == msgid) {
        ERR_EXIT("msgget error"); 
    }

    int n = 0;
    int pid = 0;
    size_t pidlen = sizeof(pid);

    struct msgbuf msg;
    memset(&msg, 0, sizeof(msg));
    while(1) {
        memset(&msg, 0, sizeof(msg));
        printf("---------\n");
        // 接收消息类型为1的消息
        if ((n = msgrcv(msgid, &msg, MAX_SIZE, 1, 0)) < 0) {
            ERR_EXIT("msgrcv error"); 
        }
        pid = *((int*)msg.mtext);
        fputs(msg.mtext + pidlen, stdout);
        printf("=========\n");
        msg.mtype = pid;
        if (msgsnd(msgid, &msg, n, 0) < 0) {
            ERR_EXIT("msgsnd error"); 
        } 
        printf("xxxxxxxxx\n");
    }

    return 0;
}
