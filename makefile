all: connmgr matcheng

CC=g++

#specify the location for include files
INSTDIR=/usr/local/bin

INCLUDE=.

# Options for development
CFLAGS=-g -Wall -ansi

connmgr: connmgr.o
	$(CC) -o $@ $^ -lrt

matcheng: matcheng.o
	$(CC) -o $@ $^ -lrt

connmgr.o: connmgr.cpp messages.h
matcheng.o: matcheng.cpp messages.h

clean:
	-rm -f *.o connmgr *~ matcheng 
