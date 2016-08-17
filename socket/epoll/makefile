.PHONY:clean all
CC=gcc
CFLAS=-Wall -g
BIN=server client
all:$(BIN)
.o:
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
