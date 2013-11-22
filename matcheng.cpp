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
using namespace std;

key_t key1,key2,key3,key4;
int msqid1,msqid2,shmid3,semid4;
struct message_msgbuf mmb;

MatchEngOBV myBooks;

/*intHandler closes queue and exits*/
void intHandler(int dummy=0){
  // closing my queue
  myBooks.Print();
  msgctl(msqid2,IPC_RMID,NULL);
  shmctl(shmid3,IPC_RMID,NULL);
  semctl(semid4,0,IPC_RMID);
  exit(0);
};

int main(){
  // set up queues
  key1 = ftok("/etc/updatedb.conf", 'b'); // key from connection manager
  msqid1 = msgget(key1, 0666 | IPC_CREAT);
  key2 = ftok("/etc/usb_modeswitch.conf", 'b'); // key to conn mgr with acks
  msqid2 = msgget(key2, 0666 | IPC_CREAT);
  key3 = ftok(METOSHKEY1,'b');
  shmid3 = shmget(key3,sizeof(struct TradeMessage),0666|IPC_CREAT);
  key4 = ftok(SEMKEY1,'b');
  semid4 = semget(key4,2,0666| IPC_CREAT);
  struct sembuf sops;
  sops.sem_num =0;
  sops.sem_op = 1;
  sops.sem_flg = 0;
  semop(semid4,&sops,1);
//  printf("msg queue id to write to: %d", msqid2);
  cout << "Initialised OBV successfully" << endl;
  myBooks.msqid = msqid2;
  myBooks.shmid = shmid3;
  myBooks.semid = semid4;
  cout << "Set key successfully" << endl;
  // load database
  //while(there is sth in the database){
    // get order from database
    // call myBook.Process(omm);
  //};
  // setting up
  printf("Ready to receive messages\n");
  // reading from message queue
  signal(SIGINT,intHandler);
  for(;;) {
//    cout << "* Matching Engine: receiving order" << endl;
    msgrcv(msqid1, &mmb, sizeof(struct OrderManagementMessage), 2, 0);
//    cout << "* Matching Engine: received order:" << endl;
    struct OrderManagementMessage omm = mmb.omm;
    printOrderManagementMessage(&omm);
//    cout << "* Matching Engine: sending order for processing" << endl;
    myBooks.Process(omm);
  };
  myBooks.Print();
  return 0;
};
