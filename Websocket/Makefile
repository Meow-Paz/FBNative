CC 		= gcc
#CFLAGS 	= -Wall -Wextra -Werror -pedantic -ggdb -DRUPIFY

INCL 	= Handshake.c
OBJECTS = Errors.o Datastructures.o Communicate.o sha1.o md5.o base64.o utf8.o
EXEC 	= Websocket

.PHONY: Websocket

all: clean Websocket

Websocket:
	$(CC) $(CFLAGS) $(INCL) -pthread $(EXEC).c Datastructures.c Errors.c base64.c md5.c sha1.c Communicate.c utf8.c -o $(EXEC) -std=c99 --static

clean:
	rm -f $(EXEC) *.o

run: all
	./$(EXEC) $(PORT)

valgrind: all
	valgrind --leak-check=full --log-file="LOG" --track-origins=yes --show-reachable=yes ./$(EXEC) $(PORT)

