# $@--目标文件
# $^--所有的依赖文件
# $<--第一个依赖文件
.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=client server
all:$(BIN)
.o:
	$(CC) $(CFLAGS) -c $< -o $@

client:client.c sockClt.o sockUtil.o
	$(CC) $(CFLAGS) $^ -o $@

server:server.c sockSrv.o sockUtil.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

clean:
	rm -rf *.dSYM *.o $(BIN)
