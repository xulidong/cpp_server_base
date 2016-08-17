#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

static void sigHandler(int sig) {
	printf("recv signal: %d\n", sig);
	alarm(1);
}

int main() {
	if(signal(SIGALRM, sigHandler) == SIG_ERR) {
		perror("signal error");	
		return 0;
	}

	alarm(1);

	while(1) {
		pause();
		printf("pause...\n");
	}

	return 0;
}
