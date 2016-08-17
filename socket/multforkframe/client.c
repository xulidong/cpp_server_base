#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "sockClt.h"

void sigHandler(int sig) {
    pid_t pid = 0;
    printf("recv sig: %d\n", sig);
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
       printf("child %d exit\n", pid); 
       fflush(stdout);
    }
}

int main() {
    signal(SIGCHLD, sigHandler);
    signal(SIGPIPE, SIG_IGN);

    int ret = 0;

    void *handle = NULL;
    ret = sockCltInit(&handle, 300, 300, 300, 1);
    if (ret != SOCK_OK) {
       printf("sockCltInit error:%d\n", ret); 
       return -1;
    }

    unsigned char *indata = (unsigned char*)"123456789\n";
    int inlen = strlen((char *)indata);
    int connfd = 0;
    pid_t pid = 0;
    int procnum = 3;
    int i = 0;
    for (i = 0; i < procnum; i++) {
       pid = fork();
       if (pid < 0) {
           printf("fork error: %d\n", errno);
           return -1;
       }
       if (0 == pid) {
           ret = sockCltGetConn(handle, "127.0.0.1", 8001, &connfd);
           if (ret != SOCK_OK) {
               printf("sockCltGetConn error:%d\n", ret); 
               return -1;
           }
            
           unsigned char outdata[1024] = {0};
           int outlen = sizeof(outdata);
           ret = sockCltSend(handle, connfd, indata, inlen);
           // 超时重发
           if (SOCK_ERROR_TIMEOUT == ret ) {
               ret = sockCltSend(handle, connfd, indata, inlen);
           } else if (ret < 0) {
               // 断线重连
               ret = sockCltGetConn(handle, "127.0.0.1", 8001, &connfd);
               if (ret != SOCK_OK) {
                   printf("sockCltSend error:%d\n", ret); 
                   return -1;
               }
           } 

           ret = sockCltRecv(handle, connfd, outdata, &outlen);
           if (ret != SOCK_OK) {
               printf("sockCltRecv error:%d\n", ret); 
               return -1;
           }

            printf("outdata:%s", outdata);
            sockCltCloseConn(connfd);
            exit(EXIT_SUCCESS);
       }
    }

    sockCltDes(handle);

    while((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("child %d exit\n", pid); 
        fflush(stdout);
    }

    return 0;
}
