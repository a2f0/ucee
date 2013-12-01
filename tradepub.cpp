#include "keys.h"
#include <error.h>
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
#include <sys/sem.h>
#include "messages.h"
#include "printing.h"
#include <cstdlib>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>
using namespace std;

// setting up socket variables
int process(TradeMessage tm);
int mysocket;
struct sockaddr_in grp;

//process function below sends multicast messages
int process(TradeMessage tm){
  struct sockaddr_in grp2;
  memset((char *) &grp2, 0, sizeof(grp));
  grp2.sin_family = AF_INET;
  grp2.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
  grp2.sin_port = htons(atoi(MULTICAST_PORT));
  ssize_t f = sendto(mysocket, &tm, sizeof(TradeMessage), 0,
                     (struct sockaddr*) &grp2, sizeof(grp2));
  if(f<0){
    fprintf(stderr,"Message Not Sent.\n");
    printf("%s",strerror(errno));
    return -1;
  };
  return 0;
};

// IPC ids
int shmid;
int sem_id;

// intHandler closes IPC's and exits
void intHandler(int dummy=0){
  // closing IPCs
  shmctl(shmid,IPC_RMID,NULL);
  semctl(sem_id,0,IPC_RMID,NULL);
  exit(0);
};

int main(){

  //shared memory setup
  key_t mykey;
  mykey = ftok(METOTPKEY1,'b'); // shared memory with matcheng
  size_t mysize = sizeof(struct TradeMessage);
  if( (shmid = shmget(mykey, mysize, 0666 | IPC_CREAT)) < 0)
    cout << "Error: shmget" << ' ' << strerror(errno) << endl;
  struct TradeMessage* tm = (struct TradeMessage*) shmat(shmid, NULL, 0);
  
  //semaphore setup
  struct sembuf sops;
  // semaphore with matcheng
  sem_id = semget(ftok(SEMKEY1,'b'), 2, 0666 | IPC_CREAT );
  if(sem_id == -1){
    perror("segment failed: ");
    exit(EXIT_FAILURE);
  }


  //multicast setup
  mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  unsigned char mc_ttl = 1;
  setsockopt(mysocket, IPPROTO_IP, IP_MULTICAST_TTL,
             (void*) &mc_ttl, sizeof(mc_ttl));
  grp.sin_family = AF_INET;
  grp.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
  grp.sin_port = htons(atoi(MULTICAST_PORT));

  sops.sem_num = 1;
  sops.sem_op = -1;
  sops.sem_flg = 0;

  signal(SIGINT,intHandler);

  int j=1;
  while(semop(sem_id, &sops, 1)!=-1){ //RESERVE SHM SEMAPHORE
    cout << "\n* trade publisher: receiving trade n. " <<j++<<" from ME";
    cout << endl;
    // send tm with big endian 64
    printTradeMsg(tm);
    tm->quantity = htobe64(tm->quantity);
    process(*tm);
    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    semop(sem_id, &sops, 1); //RELEASE SHM SEMAPHORE
    sops.sem_num=1;
    sops.sem_op=-1;
  };

  close(mysocket);
  return 0;
};
