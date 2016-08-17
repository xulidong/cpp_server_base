#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/msg.h>

/*
 * int msgrcv(int msgid, const void* msgp, size_t msgsz, long msgtyp, int msgflg);
 * struct qmsg{
 *  long mtype;
 *  char mtext[1024]
 * };
 * */

#define ERR_PRINT(msg) do { \
    printf("errno: %d\n", errno); \
    perror(msg); \
} while(0)

#define MAX_SIZE 1024

struct qmsg {
    long mtype;
    char mtext[1024];
};

int main() {
    key_t key = ftok("./msgseedfile", 'a');
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (-1 == msgid) {
        ERR_PRINT("msgget"); 
        return -1;
    }

    struct qmsg buf;
    memset(&buf, 0, sizeof(buf));
    buf.mtype = 0;// 接收消息队列的第一条
    int ret = msgrcv(msgid, &buf, MAX_SIZE, 0, IPC_NOWAIT);
    if (-1 == ret) {
        ERR_PRINT("msgrcv"); 
        return -1;
    }
    printf("recv:%s\n", buf.mtext);

    return 0;
}
