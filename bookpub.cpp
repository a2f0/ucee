//#include "matcheng.h"
#include "printing.h"
#include "messages.h"
#include "keys.h"
#include <map>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

//MatchEngOBV mybooks;
int mysocket;
struct sockaddr_in grp;

int main(){


int shmid; //for shared memory
key_t mykey; //for shared memory
mykey = ftok("/etc/smartd.conf",'b'); //for shared memory
size_t mysize = sizeof(struct OrderManagementMessage); //for shared memory
struct OrderManagementMessage* myomm = (struct OrderManagementMessage*) malloc (sizeof(OrderManagementMessage)); //for shared memory

//multicast setup---slated to remain at beginning of main function
//
mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
unsigned char mc_ttl = 1;
setsockopt(mysocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl));
memset((char *) &grp, 0, sizeof(grp));
grp.sin_family = AF_INET;
grp.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
grp.sin_port = htons(atoi(MULTICAST_PORT));
//end multicast setup
//


if( (shmid = shmget(mykey, mysize, 0666)) < 0) //for shared memory
        cout << "Error: shmget" << endl; //for shared memory






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

