.PHONY:clean all
CC=gcc
CFLAS=-Wall -g
BIN=server client
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
