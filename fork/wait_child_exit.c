#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

void func() {
	printf("test\n");
}

int main() {
	pid_t pid = 0;
	int i = 0;
	int j = 0;
	int procNum = 10;
	int loopNum = 5;

	for (i = 0; i < procNum; ++i) {
		pid = fork();
		if (pid == 0) {
			for (j = 0; j < loopNum; ++j) {
				func();	
			}	
			exit(EXIT_SUCCESS);
		}
	}

	// 父进程等待所有的子进程推出之后再退出
	i = 0;
	while(i < procNum) {
		int status;
		pid = wait(&status);
		if (WIFEXITED(status)) {
			i++;	
		}
	}
	/*
	while(1) {
		int res = wait(NULL);
		if (res == -1) {
			if (errno == EINTR) {
				continue;	
			}
			break;
		}
	}
	
	int cpid = 0;
	while((cpid = waitpid(-1, NULL, WNOHANG)) > 0) {	
		printf("child %d exit\n", cpid);
	}
	*/	

	return 0;
}
