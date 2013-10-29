all: connmgr matcheng bookpub sqlite_test

CC=gcc
CXX=g++

#specify the location for include files
INSTDIR=/usr/local/bin

INCLUDE=.

# Options for development
CXXFLAGS=-g -Wall -ansi -std=c++0x

connmgr: connmgr.o
	$(CXX) -o $@ $^ -lrt

matcheng: matcheng.o
	$(CXX) -o $@ $^ -lrt

bookpub: bookpub.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS)

sqlite3: sqlite3.o
	$(CC) -c $@ $^

sqlite_test: sqlite_test.o sqlite3.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

connmgr.o: connmgr.cpp messages.h
matcheng.o: matcheng.cpp messages.h
bookpub.o: bookpub.cpp messages.h
sqlite3.o: sqlite3.c
sqlite_test.o: sqlite3.o sqlite_test.cpp messages.h

clean:
	-rm -f *.o connmgr *~ matcheng bookpub sqlite_test
