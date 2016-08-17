CC=gcc
CFLAGS=-Wall -g
BIN=main
main:main.o add.o sub.o
	$(CC) $(CFLAGS) -o $@ $^
main.o:main.c
	$(CC) -c $^
add.o:add.c add.h
	$(CC) -c $^
sub.o:sub.c sub.h
	$(CC) -c $^

clean:
	rm -f *.o $(BIN)
