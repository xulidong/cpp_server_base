CC=gcc
CFLAGS=-Wall -g
BIN=main
OBJ=main.o add.o sub.o
main:$(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
.o:
	$(CC) -c $^

.PHONY:clean
clean:
	rm -f *.o $(BIN)
