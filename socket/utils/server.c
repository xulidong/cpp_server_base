#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include "sockSrv.h"

static void sigHandler(int sig) {
    printf("recv signale:%d\n", sig);
    while(waitpid(-1, NULL, WNOHANG) > 0) {
        printf("child exit\n");
        fflush(stdout);
    } 
}

int main() {
    signal(SIGCHLD, sigHandler);
    signal(SIGPIPE, SIG_IGN);

    int ret = 0;
    int listenfd = 0;
    ret = sockSrvInit(8001, &listenfd); 
    if (ret != 0) {
        printf("sockSrvInit error:%d\n", ret);
        return -1;
    }

    while(1) {
        int connfd = 0;
        ret = sockSrvAccept(listenfd, &connfd, 5); 
        if (ret == SOCK_ERROR_TIMEOUT) {
            printf("sockSrvAccept timeout\n");
            continue;
        }
        
        int pid = fork();
        if (pid > 0) {
            close(connfd); 
        } else if (pid == 0) {
            close(listenfd);
            unsigned char recvbuf[1024];
            int len = sizeof(recvbuf);
            while(1) {
                ret = sockSrvRecv(connfd, recvbuf, &len, 5);
                if (ret != 0) {
                   printf("sockSrvRecv error:%d\n", ret); 
                   break;
                }
                ret = sockSrvSend(connfd, recvbuf, len, 5);
                if (ret != 0) {
                   printf("sockSrvSend error:%d\n", ret); 
                   break;
                }
            }
            close(connfd);
            exit(ret);
        } else {
            close(connfd); 
            close(listenfd);
            printf("fork error:%d\n", errno);
            exit(errno);
        }
    }

    return 0;
}
