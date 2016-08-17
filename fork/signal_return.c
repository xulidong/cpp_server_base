#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

void sigHandler(int sig) {
	printf("recv signal:%d\n", sig);
	exit(0);
}

/*
 * signal 返回值类型__sighandler_t
 * */

int main() {
	if (signal(SIGINT, sigHandler) == SIG_ERR) {
		perror("signal error");
		exit(0);
	}

	while(1) {
		pause();
	} 
	
	return 0;
}
