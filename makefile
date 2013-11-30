all: connmgr matcheng bookpub tradepub rpteng tradeBot/md_receiver tradeBot/tradeBot rpt

CC=gcc
CXX=g++

#specify the location for include files
INSTDIR=/usr/local/bin

INCLUDE=.

# Options for development
CXXFLAGS=-g -Wall -ansi -std=c++0x -I/usr/bin/
CFLAGS=-Wall -g2 -std=c99 -Wunused

connmgr: connmgr.o
	$(CXX) -o $@ $^ -lpthread

matcheng: matcheng.o sqlite3.o
	$(CXX) -o $@ $^ -lrt -lpthread -ldl

sqlite3: sqlite3.o
	$(CC) -c $@ $^

bookpub: bookpub.o sqlite3.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

tradepub: tradepub.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

rpteng: rpteng.o sqlite3.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

rpt: rpt.o sqlite3.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

tradeBot/tradeBot: tradeBot/tradeBot.o
	$(CC) -o $@ $^ -lrt $(CFLAGS)

tradeBot/md_receiver: tradeBot/md_receiver.o
	$(CC) -o $@ $^ -lrt $(CFLAGS)

connmgr.o: connmgr.cpp messages.h
loopthroughmessagequeue.o: loopthroughmessagequeue.cpp messages.h
loopthrougsharedmemory.o: loopthroughsharedmemory.cpp
matcheng.o: matcheng.cpp messages.h matcheng.h orderbookview.h
sqlite3.o: sqlite3.c
bookpub.o: bookpub.cpp messages.h bookpub.h orderbookview.h
tradepub.o: tradepub.cpp messages.h
rpteng.o: rpteng.cpp messages.h
rpt.o: rpt.cpp messages.h
tradeBot/tradeBot.o: tradeBot/tradeBot.c
tradeBot/md_receiver.o: tradeBot/md_receiver.c

clean:
	-rm -f *.o connmgr *~ bookpub matcheng tradeBot/tradeBot tradeBot/md_receiver tradeBot/*.o tradepub rpteng rpt
