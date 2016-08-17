#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

static void sigHandler(int sig) {
	if (sig == SIGINT) {
		printf("recv signal: SIGINT.\n");	
		exit(0);
	} else if (sig == SIGUSR1) {
		printf("revc signal: SIGUSR1.\n");
	} else {
		printf("recv signal: %d\n", sig);
	} 
}

int main() {
	if (signal(SIGINT, sigHandler) == SIG_ERR) {
		perror("signal error");
		return 0;
	}
	if (signal(SIGUSR1, sigHandler) == SIG_ERR) {
		perror("signal error");
		return 0;
	}

	pid_t pid = fork();	
	if (pid == -1) {
		perror("fork error");
		return 0;
	} else if (pid == 0) {
		// pid = getppid();// 父进程id
		// kill(pid, SIGUSR1); // >0 向父进程发送信号
		// kill(0, SIGUSR1); // == 0 向同组进程发信号
		// kill(-1, SIGUSR1); // == -1 向所有有权限的进程发信号
		// kill(-pid, SIGUSR1); // < -1 向进程组中进程为pid的所有进程发信号
		
		pid = getpgrp();// 进程组id
		killpg(pid, SIGUSR1); // 向指定进程组发送信号

		exit(0);
	} else {
		while(1) {
		}
	}

	return 0;
}
