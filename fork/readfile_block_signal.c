#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
/*
 * 在读文件的过程中，屏蔽退出信号
 * */

#define EXIT_ERROR() {\
	printf("errno:%d\n", errno);\
	perror("error");\
	exit(EXIT_FAILURE);\
}

static void sigHandler(int sig) {

}

int main() {
	sigset_t bset;
	sigemptyset(&bset);
	sigaddset(&bset, SIGINT);
	sigaddset(&bset, SIGQUIT);
	sigprocmask(SIG_BLOCK, &bset, NULL);

	if (signal(SIGINT, sigHandler) == SIG_ERR) {
		EXIT_ERROR();
	}
	
	if (signal(SIGQUIT, sigHandler) == SIG_ERR) {
		EXIT_ERROR();
	}

	FILE* pf = fopen("a.txt", "r");
	if (pf == NULL) {
		EXIT_ERROR();
	}
	
	char c;
	do {
		c = fgetc(pf);
	} while(c != EOF);
	fclose(pf);

	sigprocmask(SIG_UNBLOCK, &bset, NULL);
			
	return 0;
}
