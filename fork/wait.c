#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

void testFun(int num) {
	printf("num: %d\n", num);
}

int main() {
	int ret = 0;
	int procNum = 5;
	int loopNum = 10;

	pid_t pid;
	for (int i = 0; i < procNum; i++) {
		pid = fork();
		if (pid == 0) {
			for(int j = 0; j < loopNum; j++) {
				testFun(j);	
			}
			exit(0);
		}	
	}
	// 检测到所有的子进程退出，父进程才推出
	while(1) {
		ret = wait(NULL);// 让父进程阻塞过程中有可能被别的信号中断，需要再做异常处理
		if (ret == -1) {
			if (errno == EINTR) {
				continue;	
			}
			break;
		}
	}
	printf("parent exit\n");

	return 0;
}
