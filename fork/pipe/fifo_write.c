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
 * 将数据写入到命名管道中
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

    int infd = open("./1.txt", O_RDONLY);
    if (-1 == infd) {
        ERR_EXIT("open infd error");
    }

    int outfd = open(pipename, O_WRONLY);
    if (-1 == outfd) {
        ERR_EXIT("open outfd error");
    }

    char buf[1024] = {0};
    int n = 0;
    while ((n = read(infd, buf, 2014)) > 0) {
       write(outfd, buf, n); 
    }
    close(infd);
    close(outfd);

    return 0;
}
