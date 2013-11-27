#include <iostream>
#include <stdio.h>
#include "messages.h"
#include <cstdlib>
#include <string>
#include <map>
#include "matcheng.h"
#include <ctime>
#include <sys/time.h>
#include <sys/msg.h>
#include <stddef.h>
#include <signal.h>
#include <sys/types.h>
#include "printing.h"
#include "keys.h"
#include "db.cpp"

using namespace std;

key_t key1,key2,key3,key4,key5;
int msqid1,msqid2,shmid3,semid4, semid5;
struct message_msgbuf mmb;

MatchEngOBV myBooks;

/*intHandler closes queue and exits*/
void intHandler(int dummy=0){
  // closing my queue
//  myBooks.Print();
  msgctl(msqid2,IPC_RMID,NULL);
  shmctl(shmid3,IPC_RMID,NULL);
  semctl(semid4,0,IPC_RMID,NULL);
  exit(0);
};

int main(){
  // set up queues
  key1 = ftok("/etc/updatedb.conf", 'b'); // key from connection manager
  msqid1 = msgget(key1, 0666 | IPC_CREAT);
  key2 = ftok("/etc/usb_modeswitch.conf", 'b'); // key to conn mgr with acks
  msqid2 = msgget(key2, 0666 | IPC_CREAT);
  key3 = ftok(METOTPKEY1,'b'); // shared memory with tradepub
  shmid3 = shmget(key3,sizeof(struct TradeMessage),0666|IPC_CREAT);
  key4 = ftok(SEMKEY1,'b'); // semaphore with tradepub
  semid4 = semget(key4,2,0666| IPC_CREAT);
  struct sembuf sops;
  sops.sem_num = 0;
  sops.sem_op = 1;
  sops.sem_flg =0;
  semop(semid4,&sops,1);
  cout << "Initialised OBV successfully" << endl;
  myBooks.msqid = msqid2;
  myBooks.shmid = shmid3;
  myBooks.semid = semid4;
  cout << "Set key successfully" << endl;
  key5 = ftok(SEMKEY3,'b'); // semaphore with bookpub
  semid5 = semget(key5,1,0666| IPC_CREAT);
//  printf("msg queue id to write to: %d", msqid2);
   // loading database
  writetodatabase = 0;
  list<Order> mylist = list<Order>(get_db("OrderBook.db","t1"));
  for(std::list<Order>::const_iterator it = mylist.begin(); it != mylist.end();it++)
      myBooks.ProcessDB(*it);
  // wait for BookPub to load database
  sops.sem_num =0;
  sops.sem_op = -1;
  sops.sem_flg = 0;
  printf("blocking due to semop\n");
  semop(semid5,&sops,1);
  // setting up
  printf("Ready to receive messages\n");
  // reading from message queue
  signal(SIGINT,intHandler);
  writetodatabase = 1;
  int j = 0;
  for(;;) {
    cout << "* Matching Engine: receiving order" << endl;
    msgrcv(msqid1, &mmb, sizeof(struct OrderManagementMessage), 2, 0);
    cout << "* Matching Engine: received order n." << j++ << "from CM"  << endl;
    struct OrderManagementMessage omm = mmb.omm;
    printOrderManagementMessage(&omm);
    cout << "* Matching Engine: sending order for processing" << endl;
    myBooks.Process(omm);
    cout << "* Order n. " << j << " processed" << endl << endl;
    };
//  myBooks.Print();
  return 0;
};
