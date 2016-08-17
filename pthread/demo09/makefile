.PHONY:all clean
BIN=mulpthread
all:$(BIN)

thread: pthread.c
	gcc -Wall -g -o $(BIN) -lpthread pthread.c

clean:
	rm -f *.o $(BIN)
