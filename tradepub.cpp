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


using namespace std;
int process(TradeMessage tm);
int mysocket;

int process(TradeMessage *tm){
printf("%s",(char*)tm->symbol);
printf("%s",(char*)tm->price);
printf("%s",(char*)tm->quantity);
//enqueue(tm);
char* arga = (char*) malloc (50*sizeof(char));
char* argp = (char*) malloc (50*sizeof(char));
sprintf(arga,"%s","myarga");
sprintf(argp,"%s","myarga");
struct sockaddr_in grp;
memset((char *) &grp, 0, sizeof(grp));
grp.sin_family = AF_INET;
grp.sin_addr.s_addr = inet_addr(arga);
grp.sin_port = htons(atoi(argp));

//3. SENDTO
//
ssize_t f = sendto(mysocket, tm, 32, 0, (struct sockaddr*) &grp, sizeof(grp));
//main error reporting:
if(f<0){
        fprintf(stderr,"Message Not Sent.\nUsage: ./sn -a 239.192.07.07 -p 1234\n");
        return -1;
}

return 0;
}



int main(){
//for(;;){
//char c;
int shmid/*,shmid2*/;
key_t mykey/*5678, mykey2=5679*/;
mykey = ftok("/etc/sensors3.conf",'i');

//size_t mysize = 27;
size_t mysize = sizeof(struct TradeMessage);
//char *shm, *s;
struct TradeMessage* tm = (struct TradeMessage*) malloc (sizeof(TradeMessage));

mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//2. SETSOCKOPT
//
unsigned char mc_ttl = 1;
setsockopt(mysocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl));

/*
if( (shmid2 = shmget(mykey2, mysize, 0666)) < 0)
	cout << "Error: shmget" << endl;

if ((shm = (char*) shmat(shmid, NULL, 0)) == (char*) -1) {
	cout << "Error: shmat" << endl;
}
*/

//while(*shm=='*'){


if( (shmid = shmget(mykey, mysize, 0666)) < 0)
	cout << "Error: shmget" << endl;

for(;;){

if ((tm = (struct TradeMessage*) shmat(shmid, NULL, 0)) == (struct TradeMessage*) -1) {
	cout << "Error: shmat" << endl;
}
//tm = shm;
printf("%s",(char*)tm->symbol);
printf("%s",(char*)tm->price);
printf("%s",(char*)tm->quantity);

process(tm);
}


//}

/*
for (s=shm; *s != NULL; s++)
	putchar(*s);
putchar('\n');

*shm = '*';
*/
//}
return 0;

}
