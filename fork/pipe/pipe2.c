#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

/*
 * 子进程写数据
 * 父进程读取其中的数据
 * 如果读端都关闭之后，还往管道中写数据，
 * 将产生SIGPIPE信号，如果不处理，将导致子进程终止
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
        sleep(0);
       close(pipefd[0]);// 子进程不需要读数据 
       write(pipefd[1], "xxxxxyyyyy", 10);
       close(pipefd[1]); 
       exit(0);
    } else {
        char buf[1024] = {0};
        close(pipefd[1]);// 父进程不需要写数据    
        close(pipefd[0]);// 同时关闭读端 
        read(pipefd[0], buf, sizeof(buf));
        printf("%s\n", buf);
        // close(pipefd[0]);
        while(1){
        }
    }

    return 0;
}
