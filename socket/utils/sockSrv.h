#ifndef __SOCKET_SRV_H__
#define __SOCKET_SRV_H__

#ifdef __cplusplus
extern 'C' {
#endif

#define SOCK_OK 0
#define SOCK_ERROR 3000

#define SOCK_ERROR_PARAM (SOCK_ERROR + 1) 
#define SOCK_ERROR_TIMEOUT (SOCK_ERROR + 2) 
#define SOCK_ERROR_PEER_CLOSED (SOCK_ERROR + 3) 
#define SOCK_ERROR_MALLOC (SOCK_ERROR + 4) 

int sockSrvInit(int port, int *listenfd); 

int sockSrvAccept(int listenfd, int *connfd, int timeout); 

int sockSrvSend(int connfd, unsigned char *indata, int inlen, int timeout);

int sockSrvRecv(int connfd, unsigned char *outdata, int *outlen, int timeout);

#ifdef __cplusplus
}
#endif

#endif
