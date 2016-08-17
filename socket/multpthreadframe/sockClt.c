#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sockClt.h"
#include "sockUtil.h"

typedef struct _SockHandle {
    int sockfd;
    int connCnt;
    int conns[100];
    int conntime;
    int sendtime;
    int recvtime;
} SockHandle;

int sockCltInit(void **handle, int conntime, int sendtime, int recvtime, int connCnt) { 
    int ret = SOCK_OK;
    if (NULL == handle || conntime < 0 || sendtime < 0 || recvtime < 0) {
        ret = SOCK_ERROR_PARAM;
        printf("sockCltInit param handle or conntime or sendtime or recvtime error: %d\n", ret);
        return ret;
    }
    
    SockHandle *tmp = (SockHandle*) malloc(sizeof(SockHandle));
    if (NULL == tmp) {
        ret = SOCK_ERROR_MALLOC;
        printf("sockCltInit malloc error: %d\n", ret); 
        return ret;
    }

    tmp->connCnt = connCnt;
    tmp->conntime = conntime;
    tmp->sendtime = sendtime;
    tmp->recvtime = recvtime;
    *handle = tmp;

    return ret;
}

int sockCltGetConn(void *handle, char *ip, int port, int *connfd) {
    int ret = SOCK_OK;
    if (NULL == handle || NULL == ip || port < 0 || port > 65535 || NULL == connfd) {
        ret = SOCK_ERROR_PARAM;
        printf("sockCltInit param handle or ip or port or conn error: %d\n", ret);
        return ret;
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        ret = errno;
        printf("sockCltInit socket error: %d\n", ret); 
        return ret;
    }

	struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(port);
	srvaddr.sin_addr.s_addr = inet_addr(ip);
     
    SockHandle *tmp = (SockHandle*)handle;
    ret = connect_timeout(sockfd, (struct sockaddr_in *)&srvaddr, tmp->conntime);
    if (ret < 0) {
        if (-1 == ret && ETIMEDOUT == ret) {
            ret = SOCK_ERROR_TIMEOUT;
            printf("sockCltGetConn connect_timeout error:%d\n", ret);
        } else {
            printf("sockCltGetConn connect_timeout error:%d\n", ret);
        }
        return ret;
    }
 
    *connfd = sockfd;

    return ret;
} 

int sockCltCloseConn(int connfd) {
    if (connfd >= 0) {
        // close(connfd);
    }
    return 0;
}

int sockCltSend(void *handle, int connfd, unsigned char *indata, int inlen) {
    int ret  = SOCK_OK;
    if (NULL == handle || NULL == indata || inlen <= 0) {
        ret = SOCK_ERROR_PARAM;
        printf("sockCltSend param handle or indata or inlen error: %d\n", ret);
        return ret;
    }

    SockHandle *tmp = (SockHandle*)handle;
    ret = write_timeout(connfd, tmp->sendtime);
    if (ret < 0) {
        if (-1 == ret && errno == ETIMEDOUT) {
            ret = SOCK_ERROR_TIMEOUT;
        }
        printf("sockCltSend write_timeout error: %d\n", ret); 
        return ret;
    }

    unsigned char *data = (unsigned char *)malloc(inlen + 4); 
    if (NULL == data) {
        ret = SOCK_ERROR_MALLOC;
        printf("sockCltSend malloc error: %d\n", ret); 
        return ret;
    }
    int len = htonl(inlen);
    memcpy(data, &len, 4);
    memcpy(data + 4, indata, inlen);
    int writeCnt = writen(connfd, data, inlen + 4);
    if (writeCnt < inlen + 4) {
        ret = writeCnt;
        printf("sockCltSend writen error: %d\n", ret); 
        free(data);
        data = NULL;
        return ret;
    }
    return SOCK_OK;
}

int sockCltRecv(void *handle, int connfd, unsigned char *outdata, int *outlen) {
    int ret = SOCK_OK;
    if (NULL == handle || NULL == outdata || NULL == outlen) {
        ret = SOCK_ERROR_PARAM;
        printf("sockCltRecv param handle or indata or outlen error: %d\n", ret);
        return ret;
    }

    SockHandle *tmp = (SockHandle*)handle;
    ret = read_timeout(connfd, tmp->recvtime);
    if (ret < 0) {
        if (-1 == ret && errno == ETIMEDOUT) {
            ret = SOCK_ERROR_TIMEOUT;
        }
        printf("sockCltRecv read_timeout error: %d\n", ret); 
        return ret;
    }

    int len = 0;
    ret = readn(connfd, &len, sizeof(len));
    if (ret == -1) {
        printf("sockCltRecv readn len error: %d\n", ret); 
        return ret;
    } else if (ret < sizeof(len)) {
        ret = SOCK_ERROR_PEER_CLOSED;
        printf("sockCltRecv readn len error: %d\n", ret); 
        return ret; 
    }

    len = ntohl(len);
    ret = readn(connfd, outdata, len);
    if (ret == -1) {
        printf("sockCltRecv readn buf error: %d\n", ret); 
        return ret;
    } else if (ret < len) {
        ret = SOCK_ERROR_PEER_CLOSED;
        printf("sockCltRecv readn buf error: %d\n", ret); 
        return ret; 
    }
    *outlen = ret;

    return SOCK_OK;
}

int sockCltDes(void *handle) {
    if (NULL != handle) {
        free(handle);
        handle = NULL;
    }
    return SOCK_OK;
}
