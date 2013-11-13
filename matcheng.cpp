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

using namespace std;

key_t key1,key2;
int msqid1,msqid2;
struct message_msgbuf mmb;

/*intHandler closes queue and exits*/
void intHandler(int dummy=0){
  // closing my queue
  msgctl(msqid2,IPC_RMID,NULL);
  exit(0);
};

int main(){
  // set up queues
  key1 = ftok("/etc/updatedb.conf", 'b');
  msqid1 = msgget(key1, 0666 | IPC_CREAT);
  key2 = ftok("/etc/usb_modeswitch.conf", 'b');
  msqid2 = msgget(key2, 0666 | IPC_CREAT);
  MatchEngOBV MyBooks;
  MyBooks.msqid = msqid2;
  // load database
  // some testing
  Order orderA = {LIMIT_ORDER,"547","Charlie","4X54",245,BUY,"MSFT","11.3",50};
  Order orderB = {LIMIT_ORDER,"314","Delta","450X",220,BUY,"MSFT","11.4",540};
  cout << orderA.order_id << orderB.order_id << endl;
  // setting up
  printf("starting matching engine\n");
  // reading from message queue
  signal(SIGINT,intHandler);
  for(;;) {
    msgrcv(msqid1, &mmb, sizeof(struct message_msgbuf), 2, 0);
    // printf("Order type: %d\n",mmb.omm.payload.order.order_type);
    // printf("Buysell: %d\n",mmb.omm.payload.order.buysell);
    MyBooks.Process(mmb.omm);
  };
  return 0;
};
