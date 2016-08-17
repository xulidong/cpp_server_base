#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

/*
 * 子进程写数据
 * 管道命令运行原理实现
 * 实例：ls | wc -w
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
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); 

        // 替换进程印象
        execlp("ls", "ls", NULL);
        // 如果替换芯的进程印象失败会执行下面这句话
        fprintf(stderr, "execute the cmd error\n");
        exit(0);
    } else {
        char buf[1024] = {0};
        close(pipefd[1]);// 父进程不需要写数据    
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);// 同时关闭读端 
        
        execlp("wc", "wc", "-w", NULL);
        wait(NULL);
    }
    return 0;
}
