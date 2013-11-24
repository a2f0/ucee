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
#include <cstdlib>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;
int process(TradeMessage tm);
int mysocket;

//SEMAPHORE STUFF:
//

/*
struct sembuf {
unsigned short sem_num;
short sem_op;
short sem_flg;
};
*/



//


int process(TradeMessage *tm){
printf("%s",(char*)tm->symbol);
printf("%s",(char*)tm->price);
printf("%s",(char*)tm->quantity);
char* arga = (char*) malloc (50*sizeof(char));
char* argp = (char*) malloc (50*sizeof(char));
sprintf(arga,"%s","128.135.164.172");
sprintf(argp,"%s","50303");
struct sockaddr_in grp;
memset((char *) &grp, 0, sizeof(grp));
grp.sin_family = AF_INET;
grp.sin_addr.s_addr = inet_addr(arga);
grp.sin_port = htons(atoi(argp));

//3. SENDTO
//
ssize_t f = sendto(mysocket, tm, 32, 0, (struct sockaddr*) &grp, sizeof(grp));
if(f<0){
        fprintf(stderr,"Message Not Sent.\nUsage: ./sn -a 239.192.07.07 -p 1234\n");
        return -1;
}

return 0;
}



int main(){


//SHM SETUP:
//
int shmid;
key_t mykey;
mykey = ftok("/etc/sensors3.conf",'b');
size_t mysize = sizeof(struct TradeMessage);
struct TradeMessage* tm = (struct TradeMessage*) malloc (sizeof(TradeMessage));
//

//SEM SETUP:
//
int sem_id;
struct sembuf sops;
/*
sops.sem_num = 1;
sops.sem_op = -1;
sops.sem_flg = 0;
*/

sem_id = semget(ftok("/etc/locale.alias",'b'), 2, 0666 | IPC_CREAT );
if(sem_id == -1){
perror("segment failed: ");
exit(EXIT_FAILURE);
}


//union semun sem_union;
//

mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//2. SETSOCKOPT
//
unsigned char mc_ttl = 1;
setsockopt(mysocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl));


//SHM GET:
//
if( (shmid = shmget(mykey, mysize, 0666)) < 0)
	cout << "Error: shmget" << endl;
//


for(;;){


//Reserve SEMAPHORE
//
//if (reserveSem(sem_id, READ_SEM) == -1)
//	printf("");

sops.sem_num = 1;
sops.sem_op = -1;
sops.sem_flg = 0;

semop(sem_id, &sops, 1); //RESERVE SEMAPHORE
//


//SHMAT
//
if ((tm = (struct TradeMessage*) shmat(shmid, NULL, 0)) == (struct TradeMessage*) -1) {
	cout << "Error: shmat" << endl;
}
//

printf("%s",(char*)tm->symbol);
printf("%s",(char*)tm->price);
printf("%s",(char*)tm->quantity);

process(tm);

sops.sem_num = 0;
sops.sem_op = 1;
sops.sem_flg = 0;

semop(sem_id, &sops, 1); //RELEASE SEMAPHORE

//if (releaseSem(sem_id, WRITE_SEM) == -1)
//	printf("");

}

return 0;

};
