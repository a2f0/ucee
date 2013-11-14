#include <stdio.h>
#include <sys/socket.h>
//#include <stdlib.h>
#include <netdb.h>
#include "messages.h"
#include <iostream>
#include <string.h>
#include <sys/msg.h>
#include <vector>
#include <map>
#include <sys/poll.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <thread>
#include <time.h>
#include <sys/time.h>



#define PORT             1337
#define SOCKET_ERROR     -1
#define QUEUE_SIZE       5
#define TRUE             1
#define FALSE            0

//using namespace std;

std::map<std::string, int> connectionmapper;

//THis is used to send the message through a System V message queue
struct message_msgbuf {
    long mtype;  /* must be positive */
    struct OrderManagementMessage omm;
};

//This should be called from a separate thread.
void readfrommatchingengine() {
    printf("Thread spawned.\n");
    /*
    key_t key2;
    int msqid2;
    key2 = ftok("/etc/usb_modeswitch.conf", 'b');
    msqid2 = msgget(key2, 0666 | IPC_CREAT);
    */
}
//
std::string isommvalid(struct OrderManagementMessage omm) {
    printf("validating message of type: %d\n", omm.type);
    if (omm.type == 0 || omm.type == 3) { //If this is a new order or a modify order verify the quantity is non-negative.
        if (omm.payload.order.quantity > 0 ) {
            char order_id[32];
            strncpy(order_id, omm.payload.order.order_id, 32);
            std::string ordr(order_id);
            printf("negative quantity detected for order %s\n", order_id);
            std::string error ("Negative quantity");
            printf("returning error\n");
            return error;
        } else {

        }
    } 
    printf("Quantity: %lu\n",  omm.payload.order.quantity );
    //the default error message
    return std::string();
}

void printCurrentTime(void)
{
     struct timeval tv;
     struct tm time_tm;
     char buffer[256];

     gettimeofday(&tv, NULL);

     localtime_r(&tv.tv_sec, &time_tm);
     strftime(buffer, sizeof(buffer), "%Y-%m-%d %T", &time_tm);
     printf("\n[%s.%06lu000]", buffer, tv.tv_usec);
}

void printFixedLengthString(const char *s, unsigned int size)
{
     char buffer[size+1];
     //memset(buffer, '\0', size + 1);
     //memcpy(buffer, s, size);
     strncpy(buffer, s, size);
     printf("%s\n", buffer);
}

void printTimestamp(unsigned long long timestamp)
{
     long s = timestamp / 1000000000;
     unsigned long us = timestamp % 1000000;
     struct tm time_tm;
     char buffer[256];

     localtime_r(&s, &time_tm);
     strftime(buffer, sizeof(buffer), "%Y-%m-%d %T", &time_tm);
     printf("%s.%06lu000", buffer, us);
}


void printOrderNak(const struct OrderNak *orderNak)
{
     printCurrentTime();
     printf(" OrderNak:");
     printf("\n  Order Id: ");
     printFixedLengthString(orderNak->order_id, ORDERID_SIZE);
     printf("\n  Timestamp: ");
     printTimestamp(orderNak->timestamp);
     printf("\n  Failure reason: ");
     printFixedLengthString(orderNak->reason, REASON_SIZE);
     printf("\n");
}

void printOrderManagementMessage(const struct OrderManagementMessage *omm)
{
     switch (omm->type) {
     case NEW_ORDER_NAK:
      printOrderNak(&omm->payload.orderNak);
      break;
     default:
      fprintf(stderr, "Unknown message type\n");
     }   
}

int main(){
    std::thread rfme(readfrommatchingengine);
    rfme.join();
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
    //int hSocket;
    //http://pic.dhe.ibm.com/infocenter/iseries/v6r1m0/index.jsp?topic=/rzab6/poll.htm
    struct pollfd fds[200];
    
    int rc;

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
        printf("could not set socket descriptor to be reusable\n");
        return -1;
    }

    int on = 1;
    //Set the socket to be non-blocking.
    rc = ioctl(hServerSocket, FIONBIO, (char *)&on);
    if (rc < 0)
    {
        printf("error set socket to be non-blocking\n");
        exit(-1);
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

    /*
    printf("Making a listen queue of %d elements\n",QUEUE_SIZE);
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("Could not listen\n");
        return 0;
    }*/

    rc = listen(hServerSocket, 32);
    if (rc < 0)
    {
        printf("error on listen.");
        exit(-1);
    }
    
    fds[0].fd = hServerSocket;
    fds[0].events = POLLIN;
    
    /*
    //Here you go.
    fd_set active_fd_set, read_fd_set; 
    //Initialize the set of active sockets.
    FD_ZERO (&active_fd_set);
    FD_SET (hServerSocket, &active_fd_set);
    //struct sockaddr_in clientname;
    //int ordernumber = 0; 
    */

    int nfds=1;
    int timeout;
    timeout = (3 * 60 * 1000);
    int current_size;
    int new_sd;
    int end_server = FALSE;
    int i;
    do {
        rc = poll(fds, nfds, timeout); 

        if (rc < 0) {
            printf("poll() failed\n");
        }
        
        if (rc == 0) {
            printf("poll() timed out\n");
        }
        current_size = nfds;
        for (i = 0; i < current_size; i++) {
            if(fds[i].revents == 0)
                continue;
            if(fds[i].revents != POLLIN) {
                printf("  Error! revents = %d\n", fds[i].revents);
                break;
            }
            if (fds[i].fd == hServerSocket) {
                printf("Listening socket is readable\n");
                do {
                    printf("Running do loop.\n");
                    new_sd = accept(hServerSocket, NULL, NULL);
                    if (new_sd < 0) {
                        if (errno != EWOULDBLOCK) {
                            printf("this should terminate");
                        }
                        printf("calling break\n");
                        break;
                    }
                    printf("new_sd: %d", new_sd);
                    printf("  New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } while (new_sd != -1);
            } else {
                printf("descriptor %d is readable\n", fds[i].fd);
                struct OrderManagementMessage omm;
                rc = recv(fds[i].fd, &omm, sizeof(omm), 0);
                std::string error = isommvalid(omm);
                printf("error returned: %s\n", error.c_str());
                //char reason[64];
                //strcpy(reason,error.c_str());
                char order_id[32];
                strncpy(order_id, omm.payload.order.order_id, 32);
                printf("time: %lld\n", (long long) time(NULL));
                if ( error.empty() ) {
                    printf("This is a valid message.\n");
                } else {  //then there was an error
                    struct OrderManagementMessage romm;
                    printf("This message is not valid. A nack needs to be sent.\n");
                    if (omm.type == 3) { //this is a modify order
                        romm.type = MODIFY_NAK; //MODIFY_NAK
                        strncpy( romm.payload.modifyNak.reason , error.c_str(),64);
                        printf("Reason set to: %s\n", romm.payload.modifyNak.reason);
                        strncpy( romm.payload.modifyNak.order_id , order_id,32);
                        romm.payload.modifyNak.timestamp = (long long) time(NULL);
                        printf("timestamp set to: %llu\n",romm.payload.modifyNak.timestamp);
                    } else if (omm.type ==0)  {
                        romm.type = NEW_ORDER_NAK; //NEW_ORDER_NAK
                        printf("configuring this as an order_nak\n");
                        strncpy( romm.payload.orderNak.reason , error.c_str(),64);
                        printf("Reason set to: %s\n", romm.payload.modifyNak.reason);
                        strncpy( romm.payload.orderNak.order_id , order_id, 32);
                        romm.payload.orderNak.timestamp = (long long) time(NULL);
                        printf("timestamp set to: %llu\n",romm.payload.orderNak.timestamp);
                    } else {
                        printf("Unknown order type failed validation\n");
                    }
                    rc = send(fds[i].fd, &romm, sizeof(romm), 0);
                    printf("=========printnack======\n");
                    printOrderManagementMessage(&romm);
                    printf("sent %d bytes through socket in NAK message\n", rc);
                }
                printf("======new order======\n");
                printf("read %d bytes through socket with file descripter %d, current size: %d\n",rc, fds[i].fd, current_size);
                printf("Message type: %d\n",omm.type);
                printf("Order type: %d\n",omm.payload.order.order_type);
                printf("Buysell: %d\n",omm.payload.order.buysell);
                printf("Account size constant: %d\n", ACCOUNT_SIZE);
                int accountsize = strlen( omm.payload.order.account ); 
                printf("Account size: %d\n", accountsize);
                printf("Quantity: %lu\n",  omm.payload.order.quantity );
                printf("Timestamp: %llu\n", omm.payload.order.timestamp );

                char acct[32];
                strncpy(acct, omm.payload.order.account, 31);
                printf("account: %s\n", acct);
                
                char usr[32];
                strncpy(usr, omm.payload.order.user, 31);
                printf("user: %s\n", usr);
                
                //printf("order_id: %s\n", order_id);
                printf("i: %d\n", i);
                std::string ordr(order_id);
                //http://www.yolinux.com/TUTORIALS/CppStlMultiMap.html
                //printf("inserting order id %s into stl map with fd: %d\n", order_id,i);
                connectionmapper.insert(std::pair<std::string,int>(ordr ,fds[i].fd ));
                printf("map size: %lu", connectionmapper.size());
                //http://beej.us/guide/bgipc/output/html/multipage/mq.html
                struct message_msgbuf mmb = {2, omm};
                msgsnd(msqid, &mmb, sizeof(struct OrderManagementMessage ), 0);
            }
        } printf("made it here\n");
    } while (end_server == FALSE);

    /* 
    for(;;) {
        read_fd_set = active_fd_set;
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            printf("select");
            return -1; 
        }
        int i;
        for (i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET (i, &read_fd_set)) {
                if (i == hServerSocket) {
                     int nw;
                     nw = hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);
                     if (nw < 0) {
                        printf("accept error\n");
                        return -1; 
                     }
                     FD_SET (nw, &active_fd_set);
                } else {
                     printf("Trying to pull data in an already existing socket.\n");
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

                        char acct[32];
                        strncpy(acct, omm.payload.order.account, 31);
                        printf("account: %s\n", acct);
                        
                        char usr[32];
                        strncpy(usr, omm.payload.order.user, 31);
                        printf("user: %s\n", usr);
                        
                        char order_id[32];
                        strncpy(order_id, omm.payload.order.order_id, 32);
                        printf("order_id: %s\n", order_id);
                        printf("i: %d\n", i);
                        std::string ordr(order_id);
                        //http://www.yolinux.com/TUTORIALS/CppStlMultiMap.html
                        printf("inserting order id %s into stl map with fd: %d\n", order_id,i);
                        connectionmapper.insert(std::pair<string,int>(ordr,i));
                        printf("map size: %d", connectionmapper.size());
                        printf("Writing to SystemV message queue\n"); 
                        //http://beej.us/guide/bgipc/output/html/multipage/mq.html
                        struct message_msgbuf mmb = {2, omm};
                        msgsnd(msqid, &mmb, sizeof(struct OrderManagementMessage ), 0);
                    }
                }
            }
        }
    }
   */
}

