#include "messages.h"
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
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <boost/lexical_cast.hpp>

int main(int argc, char* argv[]){

//Kept getopt from lab3, not sure if it will end up being relevant
//For now, to use this program you use ./bookpub -a 239.192.07.07 -p 1234
//getopt to read command line arguments
int c;
char* arga = (char*) malloc (1000*sizeof(char));
char* argp = (char*) malloc (1000*sizeof(char));

while ( (c = getopt(argc, argv, "a:p:")) != -1 ){
	switch(c){
	case 'a':
		strcpy(arga,optarg);	
	case 'p':
		strcpy(argp,optarg);
		
	}
}

//creating a dummy book message
//
struct BookMessage *msg = (struct BookMessage*) malloc (sizeof(struct BookMessage));
//the following line sets price char array to be the float 100.01
snprintf(msg->bid[0].price,10,"%f",99.01);
msg->bid[0].quantity=atoi("100");

snprintf(msg->offer[0].price,10,"%f",100.01);
msg->offer[0].quantity=atoi("100");
//printf("Price of 100.01: %s\nQuantity of 100: %llu\n",msg->bid[0].price,msg->bid[0].quantity);

//1. SOCKET CREATION
//
int mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//2. SETSOCKOPT
//
unsigned char mc_ttl = 1;
setsockopt(mysocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl));

for(;;){

//group structure, setting of address and port based on command line arguments taken from getopt
struct sockaddr_in grp;
memset((char *) &grp, 0, sizeof(grp));
grp.sin_family = AF_INET;
grp.sin_addr.s_addr = inet_addr(arga);
grp.sin_port = htons(atoi(argp));

//3. SENDTO
//
ssize_t f = sendto(mysocket, msg, 32, 0, (struct sockaddr*) &grp, sizeof(grp));
//main error reporting:
if(f<0){
	fprintf(stderr,"Message Not Sent.\nUsage: ./sn -a 239.192.07.07 -p 1234\n");
	return -1;
}
}

return 0;

}
