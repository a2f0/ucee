#include <algorithm>
#include "db.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "messages.h"
#include <cstdlib>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "db.cpp"
#include "keys.h"

// IPCs
int mysocket;
int sem_id;
int shmid;
int keydb;
int semiddb;

// intHandler closes IPCs and exits
void intHandler(int dummy=0){
  // closing IPCs
  shmctl(shmid,IPC_RMID,NULL);
  semctl(semiddb,0,IPC_RMID,NULL);
  semctl(sem_id,0,IPC_RMID,NULL);
  exit(0);
};

//this function adds a new row to the reporting table (t3)
int add_row(ReportingMessage myrm){
  int rc,c;
  char* order_to_sql = (char*) malloc (1024*sizeof(char));
  sprintf(order_to_sql,"INSERT INTO t3 VALUES ('%s',%llu,%s,%lu,'%s','%s');",
          nnstring(myrm.trademsg.symbol, SYMBOL_SIZE).c_str(),
          myrm.timestamp,nnstring(myrm.trademsg.price,PRICE_SIZE).c_str(),
          myrm.trademsg.quantity,
          nnstring(myrm.orderA.order_id,ORDERID_SIZE).c_str(),
          nnstring(myrm.orderB.order_id, ORDERID_SIZE).c_str());
  sqlite3_stmt *stmt2;
  sqlite3* mydb = create_db();
  sqlite3_open("OrderBook.db",&mydb);


  if ( (rc = sqlite3_prepare_v2(mydb,order_to_sql,-1,&stmt2,NULL))!= SQLITE_OK)
    cout << sqlite3_errmsg(mydb);
  while ( (c=sqlite3_step(stmt2)) == 100 ){;
  };
  sqlite3_finalize(stmt2);
  sqlite3_close(mydb);
  return 0;
};

int main(){
  printf("starting reporting engine...\n");

  // setting up shared memory
  key_t mykey;
  mykey = ftok(METOREKEY1,'b');
  size_t mysize = sizeof(struct ReportingMessage);
  if( (shmid = shmget(mykey, mysize, 0666 | IPC_CREAT)) < 0){
    cout << "Error: shmget" << endl;
  } else {
    printf("shmget initialized with with shmid: %d", shmid);
  }
  struct ReportingMessage* rm = (struct ReportingMessage*)shmat(shmid,NULL,0);
  //setting up semaphore for database
  keydb = ftok(SEMDB,'b');
  if ((semiddb = semget(keydb,1,0666|IPC_CREAT)) <0){
    printf("main: semget() failed\n");
    printf("%s",strerror(errno));
    exit(0);
  };
  struct sembuf sop;
  sop.sem_num =0;
  sop.sem_flg=0;

  //setting up semaphore with matching engine
  struct sembuf sops;
  sem_id = semget(ftok(METORESEM,'b'), 2, 0666 | IPC_CREAT );
  if(sem_id == -1){
    perror("segment failed: ");
    exit(EXIT_FAILURE);
  }

  sops.sem_num = 1;
  sops.sem_op = -1;
  sops.sem_flg = 0;

  signal(SIGINT,intHandler);

  int j =1;
  while(semop(sem_id, &sops, 1)!=-1){ //reserve shm semaphore
    cout << "* reporting engine: receiving reporting message n. "<< j++;
    cout << " from ME" << endl;
    printReportingMsg(rm);

    sop.sem_op = -1; // lock database semaphore
    semop(semiddb,&sop,1);
    add_row(*rm);
    sop.sem_op=1;
    semop(semiddb,&sop,1); // release database semaphore


    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    semop(sem_id, &sops, 1);//release shm semaphore
    sops.sem_num=1;
    sops.sem_op=-1;

  };
  return 0;
};
