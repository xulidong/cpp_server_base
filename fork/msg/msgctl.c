#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define ERR_PRINT(msg) do { \
    printf("errno: %d\n", errno); \
    perror(msg); \
} while(0)

int main() {
    key_t key = ftok("./msgseedfile", 'a');
    int msgid = msgget(key, 0666);
    if (-1 == msgid) {
        ERR_PRINT("msgget"); 
        return -1;
    }

    struct msqid_ds buf;
    memset(&buf, 0, sizeof(buf));
    int ret = msgctl(msgid, IPC_STAT, &buf);
    if (-1 == ret) {
        ERR_PRINT("msgctl"); 
        return -1;
    }
    printf("permission:%o\n", buf.msg_perm.mode);
    printf("byte:%lu\n", buf.msg_cbytes);
    printf("msg num:%lu\n", buf.msg_qnum);

    return 0;
}
