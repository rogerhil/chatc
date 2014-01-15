CC = gcc
CFLAGS = -g -c -Wall
LIB1 = -lm
server: protocol.o server.o
	$(CC) -g -Wall protocol.o server.o -o server $(LIB1)

server.o: server.c server.h
	$(CC) $(CFLAGS) -o server.o server.c

protocol.o: protocol.c protocol.h
	$(CC) $(CFLAGS) -o protocol.o protocol.c
        	
run: server
	./server
clean:
	\rm -rf server *.o *.c~ *.h~ Makefile~ *gz |clear

pack:
	tar cvzf server.tar.gz *.c *.h Makefile
