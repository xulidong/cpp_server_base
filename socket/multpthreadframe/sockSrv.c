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

#include "sockUtil.h"
#include "sockSrv.h"


int sockSrvInit(int port, int *listenfd) {
    int ret = 0;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
        ret = errno;
        printf("sockSrvInit socket error:%d\n", ret);
        return ret;
	}

	socklen_t optval = 1;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval));
    if (ret == -1) {
        ret = errno;
        printf("sockSrvInit setsockopt error:%d\n", ret);
        return ret;
    }

	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(port);
	srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in));
	if (ret == -1) {
        ret = errno;
        printf("sockSrvInit bind error:%d\n", ret);
        return ret;
	}

    ret = listen(sockfd, SOMAXCONN);
	if(ret == -1) {
        ret = errno;
        printf("socksrvinit listen error:%d\n", ret);
        return ret;
	}

    *listenfd = sockfd;
    return SOCK_OK;
} 

int sockSrvAccept(int listenfd, int *connfd, int timeout) {
    int ret = 0;
    ret = accept_timeout(listenfd, NULL, (unsigned int)timeout);
    if (ret <= 0) {
        if (ret == -1 && errno == ETIMEDOUT) {
            ret = SOCK_ERROR_TIMEOUT;   
        } else {
            ret = errno;
        }
        printf("socksSrvaccept accept_timeout error:%d\n", ret);
        return ret;
    }
    *connfd = ret;
    return SOCK_OK;
} 

int sockSrvSend(int connfd, unsigned char *indata, int inlen, int timeout) {
    int ret  = SOCK_OK;
    if (NULL == indata || inlen <= 0) {
        ret = SOCK_ERROR_PARAM;
        printf("sockSrvSend param indata or inlen error: %d\n", ret);
        return ret;
    }

    ret = write_timeout(connfd, timeout);
    if (ret < 0) {
        if (-1 == ret && errno == ETIMEDOUT) {
            ret = SOCK_ERROR_TIMEOUT;
        }
        printf("sockSrvSend write_timeout error: %d\n", ret); 
        return ret;
    }

    unsigned char *data = (unsigned char *)malloc(inlen + 4); 
    if (NULL == data) {
        ret = SOCK_ERROR_MALLOC;
        printf("sockSrvSend malloc error: %d\n", ret); 
        return ret;
    }
    int len = htonl(inlen);
    memcpy(data, &len, 4);
    memcpy(data + 4, indata, inlen);
    int writeCnt = writen(connfd, data, inlen + 4);
    if (writeCnt < inlen + 4) {
        ret = writeCnt;
        printf("sockSrvSend writen error: %d\n", ret); 
        free(data);
        data = NULL;
        return ret;
    }
    return SOCK_OK;
}

int sockSrvRecv(int connfd, unsigned char *outdata, int *outlen, int timeout) {
    int ret = SOCK_OK;
    if (NULL == outdata || NULL == outlen) {
        ret = SOCK_ERROR_PARAM;
        printf("sockSrvRecv param indata or outlen error: %d\n", ret);
        return ret;
    }

    ret = read_timeout(connfd, timeout);
    if (ret < 0) {
        if (-1 == ret && errno == ETIMEDOUT) {
            ret = SOCK_ERROR_TIMEOUT;
        }
        printf("sockSrvRecv read_timeout error: %d\n", ret); 
        return ret;
    }

    int len = 0;
    ret = readn(connfd, &len, sizeof(len));
    if (ret == -1) {
        printf("sockSrvRecv readn len error: %d\n", ret); 
        return ret;
    } else if (ret < sizeof(len)) {
        ret = SOCK_ERROR_PEER_CLOSED;
        printf("sockSrvRecv readn peer close, len error: %d\n", ret); 
        return ret; 
    }

    len = ntohl(len);
    ret = readn(connfd, outdata, len);
    if (ret == -1) {
        printf("sockSrvRecv readn buf error: %d\n", ret); 
        return ret;
    } else if (ret < len) {
        ret = SOCK_ERROR_PEER_CLOSED;
        printf("sockSrvRecv readn peer close, buf error: %d\n", ret); 
        return ret; 
    }
    *outlen = ret;

    return SOCK_OK;
    return 0;
}

