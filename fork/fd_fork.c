#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

int main() {
	signal(SIGCHLD, SIG_IGN);
	// 在子进程中使用fd，会使fd引用计数refcnt加1
	FILE* fd = fopen("1.txt", "w");
	if (fd == NULL) {
		perror("fopen error");
		return -1;
	}
	pid_t pid = fork();
	if (pid > 0) {
		fputs("parent\n", fd);		
		fclose(fd);
	} else if (pid == 0) {
		fputs("child\n", fd);		
		fclose(fd);
	} else {
		perror("fork");	
	}

	return 0;
}
