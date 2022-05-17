all: main clean

main: main.o
	gcc main.o -Wall -D_REENTRANT -lpthread -lrt -o main

main.o: main.c
	gcc -Wall -c main.c -o main.o

clean:
	-rm *.o