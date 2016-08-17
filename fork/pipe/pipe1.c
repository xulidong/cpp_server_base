#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/*
 * 子进程写数据
 * 父进程读取其中的数据
 * */

#define ERR_EXIT(msg) do { \
    printf("errno:%d\n", errno); \
    perror(msg); \
    exit(EXIT_FAILURE); \
} while(0)

int main() {
    int pipefd[2] = {0};
    int ret = pipe(pipefd);
    if (-1 == ret) {
        ERR_EXIT("pipe error"); 
    }

    int pid = fork();
    if (-1 == pid) {
        ERR_EXIT("fork error"); 
    }

    if (pid == 0) {
       close(pipefd[0]);// 子进程不需要读数据 
       write(pipefd[1], "xxxxxyyyyy", 10);
       close(pipefd[1]); 
    } else {
        char buf[1024] = {0};
        close(pipefd[1]);// 父进程不需要写数据    
        read(pipefd[0], buf, sizeof(buf));
        printf("%s\n", buf);
        close(pipefd[0]);
    }

    return 0;
}
