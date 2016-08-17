#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

int main() {
    // 让infile成为标准输入文件
    close(0);
    open("infile", O_RDONLY);

    // 让outfile成为标准输出文件
    close(1);
    open("outfile", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    execlp("cat", "cat", NULL);

    return 0;
}
