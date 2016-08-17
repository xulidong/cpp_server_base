#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

int main() {
	pid_t pid;
	pid_t wait_pid;
	int status;

	pid = fork();
	if (pid > 0) {
		// pid为-1, 父进程等待到任意子进程结束
		// pid小于-1, 父进程等待到其组id等于pid的绝对值的任意子进程结束
		// pid为0, 父进程等待到同一个会话组中任意子进程结束
		// pid大于0, 父进程等待制定pid子进程结束
		wait_pid = waitpid(pid, &status, 0);
		/*
		 * WIFEXITED 程序是否正常结束
		 * WEXITSTATUS 获取返回码
		 * */
		if (WIFEXITED(status)) {// wait if exited
			printf("parent catch child: %d, exit number:%d\n", wait_pid, WEXITSTATUS(status));// wait exit status
		} else if (WIFSTOPPED(status)) {
			printf("parent catch child: %d, stop number:%d\n", wait_pid, WSTOPSIG(status));// wait stop signal 
		} else if (WIFSIGNALED(status)) {
			printf("parent catch child: %d, signal number:%d\n", wait_pid, WTERMSIG(status));// wait terminal signal
		}
	} else if ( pid == 0) {
		printf("child\n");
		sleep(2);
		// exit(1);// 设置子进程的退出码为1
		abort();
	} else {
		perror("fork error.");	
	}
	return 0;
}
