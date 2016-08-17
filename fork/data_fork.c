#include <stdio.h>
#include <signal.h>
#include <errno.h>

int main() {
	int num = 0;
	printf("before fork:%p\n", &num);

	pid_t pid = fork();
	if (pid == -1) {
		perror("fork error.\n");	
		return -1;
	} 
	
	if (pid > 0) {
		printf("parent:%p\n", &num);
	} else if (pid == 0) {
		printf("child:%p\n", &num);
	}
	
	printf("after fork:%p\n", &num);
	
	return 0;
}
