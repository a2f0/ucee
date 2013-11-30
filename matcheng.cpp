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

// list of keys and ids for IPCs
key_t key1,key2,key3,key4,key5,key6,key7;
int msqid1,msqid2,shmid3,semid4, semid5, shmid6rp, semid6rp;

// message buffer to receive messages from ConnMgr
struct message_msgbuf mmb;

// Order Book View for matching engine
MatchEngOBV myBooks;

// intHandler closes queue and exits
void intHandler(int dummy=0){
  // closing IPCs
  if(shmctl(shmid3,IPC_RMID,NULL)==-1){
    printf("main: shmctl() remove id failed\n");
    printf("%s",strerror(errno));
  };
  if(semctl(semid4,0,IPC_RMID,NULL)==-1){
    printf("main: semctl() remove id failed\n");
    printf("%s",strerror(errno));
  };
  if(shmctl(shmid6rp,IPC_RMID,NULL)==-1){
    printf("main: shmctl() remove id failed\n");
    printf("%s",strerror(errno));
  };
  if(semctl(semid6rp,0,IPC_RMID,NULL)==-1){
    printf("main: semctl() remove id failed\n");
    printf("%s",strerror(errno));
  };
  if(msgctl(msqid1,IPC_RMID,NULL)==-1){
    printf("main: msgctl() remove id failed\n");
    printf("%s",strerror(errno));
  };
  if(msgctl(msqid2,IPC_RMID,NULL)==-1){
    printf("main: msqctl() remove id failed\n");
    printf("%s",strerror(errno));
  };
  // myBooks.Print();
  exit(0);
};

int main(){
  // set up queues
  key1 = ftok(CMTOMEKEY1, 'b'); // key from connection manager
  if((msqid1 = msgget(key1, 0666 | IPC_CREAT))<0){
    printf("main: msgget() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };
  
  key2 = ftok(METOCMKEY1, 'b'); // key to conn mgr with acks
  if((msqid2 = msgget(key2, 0666 | IPC_CREAT))<0){
    printf("main: msgget() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };
  
  key3 = ftok(METOTPKEY1,'b'); // shared memory with tradepub
  shmid3 = shmget(key3,sizeof(struct TradeMessage),0666|IPC_CREAT);
  shmctl(shmid3,IPC_RMID,NULL);
  if((shmid3 =shmget(key3,sizeof(struct TradeMessage),0666|IPC_CREAT))<0){
    printf("main: shmget() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };
  
  key4 = ftok(SEMKEY1,'b'); // semaphore with tradepub
  semid4 = semget(key4,2,0666| IPC_CREAT);
  semctl(semid4,0,IPC_RMID,NULL);
  if((semid4 = semget(key4,2,0666| IPC_CREAT))<0){
    printf("main: semget() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };
  
  key6 = ftok(METOREKEY1,'b'); // shared memory with reporting engine
  shmid6rp = shmget(key6,sizeof(struct ReportingMessage),0666|IPC_CREAT);
  shmctl(shmid6rp,IPC_RMID,NULL);
  if((shmid6rp=shmget(key6,sizeof(struct ReportingMessage),
                      0666|IPC_CREAT))<0){
    printf("main: shmget() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };

  key7 = ftok(METORESEM,'b'); // semaphore with reporting engine
  semid6rp = semget(key7,2,0666|IPC_CREAT);
  semctl(semid6rp,0,IPC_RMID,NULL);
  if((semid6rp = semget(key7,2,0666|IPC_CREAT))<0){
    printf("main: semget() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };
  
  struct sembuf sops;
  sops.sem_num = 0;
  sops.sem_op = 1;
  sops.sem_flg =0;
  if(semop(semid4,&sops,1)<0){
    printf("main: semop() failed\n");
    printf("%s",strerror(errno));
    exit(0);   
  };
  if(semop(semid6rp,&sops,1)<0){
    printf("main: semop() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };
  
  myBooks.msqid = msqid2;
  myBooks.shmid = shmid3;
  myBooks.semid = semid4;
  myBooks.shmidrp = shmid6rp;
  myBooks.semidrp = semid6rp;

  key5 = ftok(SEMKEY3,'b'); // semaphore with book publisher
  semid5 = semget(key5,1,0666| IPC_CREAT);
  semctl(semid5,0,IPC_RMID,NULL);
  if((semid5 = semget(key5,1,0666| IPC_CREAT))<0){
    printf("main: semget() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };
  
  // loading database
  writetodatabase = 0;
  list<Order> mylist = list<Order>(get_db("OrderBook.db","t1"));
  for(list<Order>::const_iterator it=mylist.begin();it!=mylist.end();it++)
    myBooks.Process(*it);
  // wait for BookPub to load database
  sops.sem_num =0;
  sops.sem_op = -1;
  sops.sem_flg = 0;
  printf("\n* blocking while we wait for bookpub to read database\n");
  if(semop(semid5,&sops,1)<0){
    printf("main: semop() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };

  // setting up
  printf("\n* ready to receive messages\n");
  // reading from message queue
  signal(SIGINT,intHandler);
  writetodatabase = 1;
  int j = 1;
  while(msgrcv(msqid1,&mmb,sizeof(struct OrderManagementMessage),2,0)!=-1
        && j < 11){
    cout << "\n* matching engine: receiving order n. "<< j++<< " from CM";
    cout << endl;
    struct OrderManagementMessage omm = mmb.omm;
    printOrderManagementMessage(&omm);
    myBooks.Process(omm);
  };
  // myBooks.Print();
  return 0;
};
