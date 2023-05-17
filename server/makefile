CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -pthread

all: dbclient dbserver

dbclient: dbclient.c
	$(CC) $(CFLAGS) dbclient.c -o dbclient

dbserver: dbserver.c
	$(CC) $(CFLAGS) dbserver.c -o dbserver

clean:
	rm -f dbclient dbserver
