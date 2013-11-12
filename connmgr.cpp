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
                printf("read %d bytes through socket with file descripter %d, current size: %d\n",rc, fds[i].fd, current_size);
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
