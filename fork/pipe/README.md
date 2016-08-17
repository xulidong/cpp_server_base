匿名管道应用的一个限制是只能用在具有亲缘关系的进程之间通信
FIFO文件可以用在不同进程之间交换数据，通常被称为命名管道

命名管道创建
1 命令：mkfifo filename
2 程序: int mkfifo(char * filename, mode t)
