#ifndef __SOCKET_CLT_H__
#define __SOCKET_CLT_H__

#ifdef __cplusplus
extern 'C' {
#endif

#define SOCK_OK 0
#define SOCK_ERROR 3000

#define SOCK_ERROR_PARAM (SOCK_ERROR + 1) 
#define SOCK_ERROR_TIMEOUT (SOCK_ERROR + 2) 
#define SOCK_ERROR_PEER_CLOSED (SOCK_ERROR + 3) 
#define SOCK_ERROR_MALLOC (SOCK_ERROR + 4) 

int sockCltInit(void **handle, int conntime, int sendtime, int recvtime, int connCnt); 

int sockCltGetConn(void *handle, char *ip, int port, int *conn); 

int sockCltCloseConn(int connfd);

// int sockCltPutConn(int* conn);

int sockCltSend(void *handle, int connfd, unsigned char *indata, int inlen);

int sockCltRecv(void *handle, int connfd, unsigned char *outdata, int *outlen);

int sockCltDes(void *handle);

#ifdef __cplusplus
}
#endif

#endif
