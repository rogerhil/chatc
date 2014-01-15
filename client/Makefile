CC = gcc -lpthread
CFLAGS = -g -c -Wall
LIB1 = -lm
client: protocol.o client.o
	$(CC) -g -Wall protocol.o client.o -o client $(LIB1)

client.o: client.c client.h
	$(CC) $(CFLAGS) -o client.o client.c

protocol.o: protocol.c protocol.h
	$(CC) $(CFLAGS) -o protocol.o protocol.c
        	
run: client
	./client
clean:
	\rm -rf client *.o *.c~ *.h~ Makefile~ *gz |clear

pack:
	tar cvzf client.tar.gz *.c *.h Makefile
