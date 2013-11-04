all: connmgr bookpub sqlite_test loopthroughmessagequeue tradeBot/md_receiver tradeBot/tradeBot

CC=gcc
CXX=g++

#specify the location for include files
INSTDIR=/usr/local/bin

INCLUDE=.

# Options for development
CXXFLAGS=-g -Wall -ansi -std=c++0x
CFLAGS=-Wall -g2 -std=c99 -Wunused

connmgr: connmgr.o
	$(CXX) -o $@ $^ -lrt

loopthroughmessagequeue: loopthroughmessagequeue.o
	$(CXX) -o $@ $^ -lrt

#matcheng/matcheng: matcheng/matcheng.o
#	$(CXX) -o $@ $^ -lrt

bookpub: bookpub.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS)

sqlite3: sqlite3.o
	$(CC) -c $@ $^

sqlite_test: sqlite_test.o sqlite3.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

tradeBot/tradeBot: tradeBot/tradeBot.o
	$(CC) -o $@ $^ -lrt $(CFLAGS)

tradeBot/md_receiver: tradeBot/md_receiver.o
	$(CC) -o $@ $^ -lrt $(CFLAGS)

connmgr.o: connmgr.cpp messages.h
loopthroughmessagequeue.o: loopthroughmessagequeue.cpp messages.h
#matcheng/matcheng.o: matcheng.cpp messages.h
bookpub.o: bookpub.cpp messages.h
sqlite3.o: sqlite3.c
sqlite_test.o: sqlite3.o sqlite_test.cpp messages.h
tradeBot/tradeBot.o: tradeBot/tradeBot.c
tradeBot/md_receiver.o: tradeBot/md_receiver.c

clean:
	-rm -f *.o connmgr *~ bookpub sqlite_test loopthroughmessagequeue tradeBot/tradeBot tradeBot/md_receiver tradeBot/*.o
