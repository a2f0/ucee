all: connmgr matcheng bookpub tradepub db loopthroughmessagequeue tradeBot/md_receiver tradeBot/tradeBot

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

loopthroughmessagequeue: loopthroughmessagequeue.o
	$(CXX) -o $@ $^ -lrt

matcheng: matcheng.o
	$(CXX) -o $@ $^ -lrt

sqlite3: sqlite3.o
	$(CC) -c $@ $^

db: db.o sqlite3.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

bookpub: bookpub.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

tradepub: tradepub.o
	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

tradeBot/tradeBot: tradeBot/tradeBot.o
	$(CC) -o $@ $^ -lrt $(CFLAGS)

tradeBot/md_receiver: tradeBot/md_receiver.o
	$(CC) -o $@ $^ -lrt $(CFLAGS)

#shm_cl: shm_cl.o
#	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

#shm_sv: shm_sv.o
#	$(CXX) -o $@ $^ -lrt $(CXXFLAGS) -lpthread -ldl

#shm-boost: shm-boost.o
#	g++ -I/usr/include/boost shm-boost.cpp -o shm-boost -lpthread -lrt

connmgr.o: connmgr.cpp messages.h
loopthroughmessagequeue.o: loopthroughmessagequeue.cpp messages.h
matcheng.o: matcheng.cpp messages.h matcheng.h orderbookview.h
sqlite3.o: sqlite3.c
db.o: sqlite3.o db.cpp messages.h
bookpub.o: bookpub.cpp messages.h
tradepub.o: tradepub.cpp messages.h
tradeBot/tradeBot.o: tradeBot/tradeBot.c
tradeBot/md_receiver.o: tradeBot/md_receiver.c
#shm_cl.o: shm_cl.cpp messages.h
#shm_sv.o: shm_sv.cpp messages.h

clean:
	-rm -f *.o connmgr *~ bookpub matcheng loopthroughmessagequeue tradeBot/tradeBot tradeBot/md_receiver tradeBot/*.o db tradepub
