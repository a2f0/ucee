all: connmgr matcheng bookpub

CC=g++

#specify the location for include files
INSTDIR=/usr/local/bin

INCLUDE=.

# Options for development
CXXFLAGS=-g -Wall -ansi -std=c++0x

connmgr: connmgr.o
	$(CC) -o $@ $^ -lrt

matcheng: matcheng.o
	$(CC) -o $@ $^ -lrt

bookpub: bookpub.o
	$(CC) -o $@ $^ -lrt $(CXXFLAGS)

connmgr.o: connmgr.cpp messages.h
matcheng.o: matcheng.cpp messages.h
bookpub.o: bookpub.cpp messages.h

clean:
	-rm -f *.o connmgr *~ matcheng bookpub
