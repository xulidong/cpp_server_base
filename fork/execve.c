#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main() {
	pid_t pid = vfork();
	if (pid > 0) {
		printf("parent\n");
	} else if (pid == 0) {
		printf("child\n");
		char* const argv[] = {"ls", "-l", NULL};
		int ret = execve("/bin/ls", argv, NULL);
		if (ret == -1) {
			perror("execve");
		}
		exit(0);
	} else {
		perror("vfork");
	}

	return 0;
}
