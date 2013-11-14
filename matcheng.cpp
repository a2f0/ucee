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
using namespace std;

key_t key1,key2;
int msqid1,msqid2;
struct message_msgbuf mmb;

MatchEngOBV myBooks;

/*intHandler closes queue and exits*/
void intHandler(int dummy=0){
  // closing my queue
  myBooks.Print();
  msgctl(msqid2,IPC_RMID,NULL);
  exit(0);
};

int main(){
  // set up queues
  key1 = ftok("/etc/updatedb.conf", 'b');
  msqid1 = msgget(key1, 0666 | IPC_CREAT);
  key2 = ftok("/etc/usb_modeswitch.conf", 'b');
  msqid2 = msgget(key2, 0666 | IPC_CREAT);
  cout << "Initialised OBV successfully" << endl;
  myBooks.msqid = msqid2;
  cout << "Set key successfully" << endl;
  cout << true << endl;
  cout << false << endl;
  // load database
  
  // setting up
  printf("starting matching engine\n");
  // reading from message queue
  signal(SIGINT,intHandler);
  for(int m = 0; m < 10;m++) {
    cout << "Receiving order:" << endl;
    msgrcv(msqid1, &mmb, sizeof(struct message_msgbuf), 2, 0);
    // cout << "Received order" << endl;
    struct OrderManagementMessage omm = mmb.omm;
    printOrderManagementMessage(&omm);
    myBooks.Process(omm);
  };
  return 0;
};
