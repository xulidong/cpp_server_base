#include <stdio.h>
#include <string.h>

#include "sockClt.h"

int main() {
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

    sockCltDes(handle);

    return 0;
}
