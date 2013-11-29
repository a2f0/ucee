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

using namespace std;
int process(TradeMessage tm);
int mysocket;

int process(TradeMessage tm){
//char* arga = (char*) malloc (50*sizeof(char));
//char* argp = (char*) malloc (50*sizeof(char));
//sprintf(arga,"%s", MULTICAST_ADDRESS);
//sprintf(argp,"%s", MULTICAST_PORT);
struct sockaddr_in grp;
memset((char *) &grp, 0, sizeof(grp));
grp.sin_family = AF_INET;
grp.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
grp.sin_port = htons(atoi(MULTICAST_PORT));
ssize_t f = sendto(mysocket, &tm, 32, 0, (struct sockaddr*) &grp, sizeof(grp));
if(f<0){
        fprintf(stderr,"Message Not Sent.\nUsage: ./sn -a 239.192.07.07 -p 1234\n");
        return -1;
}

return 0;
}



int shmid;
int sem_id;

void intHandler(int dummy=0){
  // closing IPCs
  shmctl(shmid,IPC_RMID,NULL);
  semctl(sem_id,IPC_RMID,NULL);
  exit(0);
};




int main(){

//SHM SETUP:
//
key_t mykey;
mykey = ftok(METOTPKEY1,'b'); // shared memory with matcheng
size_t mysize = sizeof(struct TradeMessage);
if( (shmid = shmget(mykey, mysize, 0666 | IPC_CREAT)) < 0)
	cout << "Error: shmget" << ' ' << strerror(errno) << endl;
struct TradeMessage* tm = (struct TradeMessage*) shmat(shmid, NULL, 0);



//SEM SETUP:
//
struct sembuf sops;
// semaphore with matcheng
sem_id = semget(ftok(SEMKEY1,'b'), 2, 0666 | IPC_CREAT );
if(sem_id == -1){
perror("segment failed: ");
exit(EXIT_FAILURE);
}



mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
unsigned char mc_ttl = 1;
setsockopt(mysocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl));

sops.sem_num = 1;
sops.sem_op = -1;
sops.sem_flg = 0;

signal(SIGINT,intHandler);

while(semop(sem_id, &sops, 1)!=-1){ //RESERVE SEMAPHORE

if ((tm = (struct TradeMessage*) shmat(shmid, NULL, 0)) == (struct TradeMessage*) -1) {
        cout << "Error: shmat" << endl;
}
 

  printTradeMsg(tm);

//process(*tm);




sops.sem_num = 0;
sops.sem_op = 1;
sops.sem_flg = 0;
semop(sem_id, &sops, 1); //RELEASE SEMAPHORE
sops.sem_num=1;
sops.sem_op=-1;

};
//shmctl(shmid, IPC_RMID, struct shmid_ds * buf);
return 0;

};
