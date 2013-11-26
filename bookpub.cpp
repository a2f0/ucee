#include "bookpub.h"
#include "printing.h"
#include "messages.h"
#include "keys.h"
#include "db.cpp"
#include <map>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <stddef.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

int mysocket;
struct sockaddr_in grp;

key_t key1, key2, key3, key4,key5;
int msqid1, msqid2, shmid3, semid4,semid5;
struct message_msgbuf mmb;

BookPubOBV myBooks;

void intHandler(int dummy=0){
  // closing IPCs
  myBooks.Print();
  shmctl(shmid3, IPC_RMID,NULL);
  semctl(semid4,0, IPC_RMID,NULL);
};

int main(){
  
  
mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
unsigned char mc_ttl = 1;
setsockopt(mysocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl));
//memset((char *) &grp, 0, sizeof(grp));
myBooks.grp.sin_family = AF_INET;
myBooks.grp.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
myBooks.grp.sin_port = htons(atoi(MULTICAST_PORT));
myBooks.mysocket=mysocket;
//myBooks.grp=grp;




  key3 = ftok(CMTOBPKEY1,'b'); //for shared memory
  size_t mysize = sizeof(struct OrderManagementMessage); //for shared memory
  shmid3 = shmget(key3, mysize, 0666|IPC_CREAT);
  key4 = ftok(SEMKEY2,'b'); // semaphore w/ connection manager
  semid4 = semget(key4,2,0666|IPC_CREAT);
  key5 = ftok(SEMKEY3,'b');
  semid5= semget(key5,1,0666|IPC_CREAT);
  struct OrderManagementMessage* ptr
    = (struct OrderManagementMessage*) shmat(shmid3,NULL,0);
  cout << "Initialized BookPubOBV successfully" << endl;
  cout << "Set keys successfully" << endl;
  // loading database
  list<Order> mylist = list<Order>(get_db("OrderBook.db","t1"));
  for (std::list<Order>::const_iterator it = mylist.begin(); it!=mylist.end();++it)
    myBooks.ProcessDB(*it);
  // allow Matching engine to continue
  struct OrderManagementMessage omm;
  struct sembuf sops;
  sops.sem_num = 0;
  sops.sem_op = 1;
  sops.sem_flg = 0;
  semop(semid5,&sops,1);
  // setting up;
  printf("Ready to receive messages\n");
  //reading from shared memory
  signal(SIGINT,intHandler);
  sops.sem_num = 1;
  sops.sem_op = -1;
  while(semop(semid4,&sops,1)!=-1){
    cout << "* Book Publisher: received order:" << endl;
    printOrderManagementMessage(ptr);
    cout << "* Book Publisher: sending order for processing" << endl;
    memcpy(&omm,ptr,sizeof(omm));
    myBooks.Process(omm);
    sops.sem_num =0;
    sops.sem_op =1;
    semop(semid4,&sops,1);
    sops.sem_num = 1;
    sops.sem_op = -1;
  };
  myBooks.Print();
  return 0;
};

//multicast setup---slated to remain at beginning of main function
/*  struct OrderManagementMessage* myomm
    = (struct OrderManagementMessage*) malloc (mysize); //shm buff
mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
unsigned char mc_ttl = 1;
setsockopt(mysocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl));
memset((char *) &grp, 0, sizeof(grp));
grp.sin_family = AF_INET;
grp.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
grp.sin_port = htons(atoi(MULTICAST_PORT));
//end multicast setup
//







for(;;){

if ((myomm = (struct OrderManagementMessage*) shmat(shmid, NULL, 0)) == (struct OrderManagementMessage*) -1) { //for shared memory
        cout << "Error: shmat" << endl;
}

//multicast sending---slated to go in bookpub.h
//
ssize_t f = sendto(mysocket, myomm, 32, 0, (struct sockaddr*) &grp, sizeof(grp));
if(f<0){
	fprintf(stderr,"Message Not Sent.\nUsage: ./sn -a 239.192.07.07 -p 1234\n");
	return -1;
	}
//end multicast sending
//
}


return 0;

}
*/
