all: client server clean

client: client.o
	gcc client.o -Wall -D_REENTRANT -lpthread -lrt -o client

client.o: client.c
	gcc -Wall -c client.c -o client.o


server: server.o
	gcc server.o -Wall -D_REENTRANT -lpthread -lrt -o server

server.o: server.c
	gcc -Wall -c server.c -o server.o

clean:
	-rm *.o