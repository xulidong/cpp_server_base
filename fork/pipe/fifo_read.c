#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * 利用命名管道
 * 拷贝1.txt的数据到2.txt
 * 从管道中读取数据，写入到2.txt中
 * */

#define ERR_EXIT(msg) do { \
    printf("errno:%d\n", errno); \
    perror(msg); \
    exit(EXIT_FAILURE); \
} while(0)

void sigHandler(int sig) {
    printf("signal:%d\n", sig);
}

int main() {
    signal(SIGPIPE, sigHandler);

    // 创建一个命名管道
    char* pipename = "fifofile";
    mkfifo(pipename, 0644);

    int outfd = open("./2.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (-1 == outfd) {
        ERR_EXIT("open outfd error");
    }
    
    int infd = open(pipename, O_RDONLY);
    if (-1 == infd) {
        ERR_EXIT("open infd error");
    }

    char buf[1024] = {0};
    int n = 0;
    while ((n = read(infd, buf, 2014)) > 0) {
       write(outfd, buf, n); 
    }
    close(infd);
    close(outfd);
    unlink(pipename);

    return 0;
}
