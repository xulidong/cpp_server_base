#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "sockUtil.h"

#define CLT_SIZE 2048

int main() {
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	socklen_t optval = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval));

	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8001);
	srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(listenfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in));
	listen(listenfd, SOMAXCONN); 

    int kq = kqueue();
    struct kevent kqfds[1];
    EV_SET(&kqfds[0], listenfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq, kqfds, 1, NULL, 0, NULL);
    
    int i = 0;
    int j = 0;
    int nready = 0;
    struct kevent events[CLT_SIZE];
    while(1) {
        nready = kevent(kq, NULL, 0, events, CLT_SIZE, NULL);
        for (i = 0; i < nready; ++i) {
            int fd = events[i].ident;
            int num = events[i].data;
            if (fd == listenfd) {
                for (j = 0; j < num; ++j) {
                    int client = accept(fd, NULL, NULL);
                    struct kevent kqfds[1];
                    EV_SET(&kqfds[0], client, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    kevent(kq, kqfds, 1, NULL, 0, NULL);
                }
            } else {
                char recvbuf[1024] = {0}; 
                int ret = readline(fd, recvbuf, sizeof(recvbuf));
                if (ret == -1) {
                    break;
                } else if (ret == 0) {
                    printf("client close\n");
                    close(fd);
                }
                fputs(recvbuf, stdout);
                writen(fd, recvbuf, strlen(recvbuf));
            }
        }
    }
    close(listenfd);

	return 0;
}
