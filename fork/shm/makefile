.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=write read
all:$(BIN)
.o:
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
