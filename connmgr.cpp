#include <stdio.h>
#include <sys/socket.h>
//#include <stdlib.h>
#include <netdb.h>
#include "messages.h"
#include <iostream>
#include <string.h>
#include <sys/msg.h>

#define PORT            1337
#define SOCKET_ERROR    -1
#define QUEUE_SIZE      5 

using namespace std;

//THis is used to send the message through a System V message queue
struct message_msgbuf {
    long mtype;  /* must be positive */
    struct OrderManagementMessage omm;
};


int main(){
    key_t key;
    key = ftok("/etc/updatedb.conf", 'b');
    int msqid;
    msqid = msgget(key, 0666 | IPC_CREAT); 
    printf("msgqid is equal to:%d\n",msqid); 
    
    printf("Starting connection manager\n"); 
    int hServerSocket;
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in Address;
    int nAddressSize=sizeof(struct sockaddr_in);
    int hSocket;
 
    if(hServerSocket == SOCKET_ERROR) {
    	printf("\nCould not make a socket\n");
	return -1;
    }
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(PORT);
    Address.sin_family=AF_INET;
   
    //Allow socket reuse 
    int yes = 1;
    if (setsockopt(hServerSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        return -1;
    }
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR) {
        printf("\nCould not connect to host\n");
        return 0;
    }
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port) );
    printf("Server\n\
             sin_family        = %d\n\
             sin_addr.s_addr   = %d\n\
             sin_port          = %d\n"
             , Address.sin_family
             , Address.sin_addr.s_addr
             , ntohs(Address.sin_port)
    );

    printf("Making a listen queue of %d elements\n",QUEUE_SIZE);
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("Could not listen\n");
        return 0;
    }
	
     //Here you go.
    fd_set active_fd_set, read_fd_set; 
    //Initialize the set of active sockets.
    FD_ZERO (&active_fd_set);
    FD_SET (hServerSocket, &active_fd_set);
    //struct sockaddr_in clientname;
  
    //int ordernumber = 0; 
    for(;;) {
        //printf("Entered the loop.\n");
        read_fd_set = active_fd_set;
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            printf("select");
            return -1; 
        }
        int i;
        //size_t size;
	for (i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET (i, &read_fd_set)) {
		if (i == hServerSocket) {
                     /* Connection request on original socket. */
                     int nw;
		     //size = sizeof (clientname);
                     nw = hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);
                     //printf("\nReceived an inbound connection via select()...\n");
                     if (nw < 0) {
			printf("accept error\n");
                        return -1; 
                     }
                     FD_SET (nw, &active_fd_set);
		} else {
                     printf("Trying to pull data in an already existing socket\n");
                     //ordernumber++;
    	      	     struct OrderManagementMessage omm;
                     int bytes_read;
                     bytes_read = read(i,&omm,sizeof(omm));
                     if (bytes_read == 0) {
        	         close (i);
        	         FD_CLR (i, &active_fd_set); 
		     } else {
                         printf("read %d bytes through socket with file descripter %d\n",bytes_read, i);
                         printf("======new order======\n");
                         printf("Message type: %d\n",omm.type);
                         printf("Order type: %d\n",omm.payload.order.order_type);
                         printf("Buysell: %d\n",omm.payload.order.buysell);
                         printf("Account size constant: %d\n", ACCOUNT_SIZE);
                         int accountsize = strlen( omm.payload.order.account ); 
                         printf("Account size: %d\n", accountsize);
                         printf("Quantity: %lu\n",  omm.payload.order.quantity );
                         printf("Timestamp: %llu\n", omm.payload.order.timestamp );
                         printf("Account: %s\n",omm.payload.order.account);
                         printf("Writing to SystemV message queue\n"); 
                         //http://beej.us/guide/bgipc/output/html/multipage/mq.html
                         struct message_msgbuf mmb = {2, omm};
                         msgsnd(msqid, &mmb, sizeof(struct OrderManagementMessage ), 0);
                     //printf("Account: %s\n",omm.payload.order.account);
                     //printf("Account: %s\n",omm.payload.order.account[ACCOUNT_SIZE]);
                     //printf("Buysell: %d, Account: %s\n", omm.payload.order.buysell, omm.payload.order.account);
                    }
		}
            }
        }	
    }
}
