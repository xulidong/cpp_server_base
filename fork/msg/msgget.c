#include <stdio.h>
#include <errno.h>
#include <sys/msg.h>

/*
 * 删除消息队列命令:ipcrm -q msgid
 * 消息队列
 * msgget 打开消息队列，第一个参数消息队列名，第二个参数权限|flags
 * */

int main() {
    // 使用文件生成消息队列名，用于在多个进程之间，统一消息队列名，
    // msgseedfile必须存在
    key_t key = ftok("./msgseedfile", 'a');// 
    // 打开消息队列，不存在则打开失败，errno == ENOENT
    // int msgid = msgget(0x1234, 0666); 

    // 打开消息队列，不存在则创建
    int msgid = msgget(key, 0466 | IPC_CREAT); 
     
    // 打开消息队列，不存在则创建，存在则报错，errno == EEXIST
    // int msgid = msgget(0x1234, 0666 | IPC_CREAT | IPC_EXCL); 
    
    // 使用IPC_PRIVATE之后，只能在有血缘关系的进程之间使用,而且后面的标志位失效，每次都创建新的队列，返回的msgid夜不一样
    // int msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT | IPC_EXCL); 
    if (-1 == msgid) {
        printf("errno: %d\n", errno);
        perror("msgid error"); 
        if (ENOENT == errno) {
            printf("key not exist\n");
        } else if (EEXIST == errno) {
            printf("key not exist\n");
        }
        return -1;
    }
    
    // 用高权限获取低权限创建的队列会报错，errno == EACCES
    msgid = msgget(0x1234, 0666);
    if (-1 == msgid) {
        printf("errno: %d\n", errno);
        perror("msgid error"); 
    }
    return 0;
}
