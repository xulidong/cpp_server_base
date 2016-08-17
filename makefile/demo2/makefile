#gcc -c add.c
#gcc -c sub.c
#gcc -c main.c
#gcc -o main main.o add.o sub.o
main:main.o add.o sub.o
	gcc -o main main.o add.o sub.o
main.o:main.c
	gcc -c main.c
add.o:add.c add.h
	gcc -c add.c
sub.o:sub.c sub.h
	gcc -c sub.c
clean:
	rm -f *.o main
