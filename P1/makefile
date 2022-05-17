all: main clean

main: main.o
	cc main.o -Wall -Wextra -o main

main.o: main.c
	cc -Wall -c main.c -o main.o

clean:
	-rm *.o