#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/*
 * system 调用"/bin/sh -c command"执行命令，阻塞当前进程直到command命令执行完成
 * 无法启动shell运行命令，返回127；
 * 出现不能执行system调用的其他错误，返回－1；
 * 顺利执行，返回命令的退出码。
 * */

int mySystem(const char* command) {
	int status;
	pid_t pid = fork();
	if (pid < 0) {
		status = -1;
	} else if (pid == 0) {
		execl("/bin/sh", "sh", "-c", command, NULL);
		exit(127);
	} else {
		while(waitpid(pid, &status, 0) < 0) {
			if (errno == EINTR) {
				continue;
			}
			status = -1;
			break;
		}	
	}
	return status;
}

int main() {
	mySystem("ls -l");	
	return 0;
}
