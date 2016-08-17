#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/msg.h>

/*
 * int msgsnd(int msgid, const void* msgp, size_t msgsz, int msgflg);
 * struct qmsg{
 *  long mtype;
 *  char mtext[1024]
 * };
 * */

#define ERR_PRINT(msg) do { \
    printf("errno: %d\n", errno); \
    perror(msg); \
} while(0)

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
    memset(&buf, 0, sizeof(struct qmsg));
    buf.mtype = 1;
    strcpy(buf.mtext, "xxxxxdafagdsfdsasdaaa");
    int ret = msgsnd(msgid, &buf, 10, IPC_NOWAIT);
    if (-1 == ret) {
        ERR_PRINT("msgget"); 
        return -1;
    }

    return 0;
}
