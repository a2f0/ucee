#include <stdio.h>
#include <sys/socket.h>
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
#include <signal.h>
#include "printing.h"
#include <errno.h>
#include "keys.h"
//#include <sys/sem.h>
#include <mutex>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PORT             1338
#define SOCKET_ERROR     -1
#define QUEUE_SIZE       5
#define TRUE             1
#define FALSE            0

//#define SEMKEYPATH "/home"       /* Path used on ftok for semget key  */
//#define SEMKEYID 1              /* Id used on ftok for semget key    */
#define NUMSEMS 2

//A decent amount of this code was stolen from here:
//http://pic.dhe.ibm.com/infocenter/iseries/v6r1m0/index.jsp?topic=/rzab6/poll.htm
//http://www.yolinux.com/TUTORIALS/CppStlMultiMap.html
//http://beej.us/guide/bgipc/output/html/multipage/mq.html
//http://publib.boulder.ibm.com/infocenter/iseries/v5r3/index.jsp?topic=%2Fapis%2Fapiexusmem.htm

std::map<std::string, int> connectionmapper;
std::mutex writetoken;

long long int found = 0;
long long int notfound = 0;
long long int receivedfromtradebots = 0;
long long int copiedthroughsharedmemory = 0;

//This is used to send the message through a System V message queue
//struct message_msgbuf {
//    long mtype;  /* must be positive */
//    struct OrderManagementMessage omm;
//};

int semid;
int shmid;
int msqid;
int msqid2;
struct shmid_ds shmid_struct;
struct OrderManagementMessage* shm;
  
void my_handler(int s){
    printf("caught signal %d\n",s);
    int rc;
    rc = semctl( semid, 1, IPC_RMID );
    if (rc==-1)
    {
        printf("main: semctl() remove id failed\n");
    }
    rc = shmdt(shm);
    if (rc==-1)
    {
        printf("main: shmdt() failed\n");
    }
    rc = shmctl(shmid, IPC_RMID, &shmid_struct);
    if (rc==-1)
    {
        printf("main: shmctl() failed\n");
    }
    if ((msgctl(msqid2,IPC_RMID,NULL)) == -1)
    {
        printf("error removing message queue with id %d: %s\n", msqid2, strerror(errno));
    }
    if ((msgctl(msqid,IPC_RMID,NULL)) == -1)
    {
        printf("error removing message queue with id %d: %s\n", msqid, strerror(errno));
    }
    
    printf("******** begin connection manager performance summary ********\n");
    printf("found (and acknowledged) for reverse routing (to tradebot): %llu\n", found);
    printf("not found for reverse routing (to tradebot): %llu\n",notfound);
    printf("total messages received from trade bots: %llu\n",receivedfromtradebots);
    printf("messages copied through shared memory: %llu", copiedthroughsharedmemory);
    printf("******** end connection manager performance summary ********\n");
    exit(1);
}

//This should be called from a separate thread.
void readfrommatchingengine() {

    printf("thread to read from matching engine system v message queue spawned.\n");
    key_t key2;
    key2 = ftok(METOCMKEY1, 'b');
    msqid2 = msgget(key2, 0666 | IPC_CREAT);
    printf("message queue id after creation: %d\n", msqid2);
    int rc = msgctl(msqid2,IPC_RMID,NULL);
    if (rc == -1 ) {
        printf("error removing message queue with id %d: %s\n", msqid2, strerror(errno));
    }
    msqid2 = msgget(key2, 0666 | IPC_CREAT);
    printf("getting ready to read order acknowledgement messages from message queue...\n");
    //char *order_id
    char order_id[33];
    order_id[32] = '\0';
    
    struct message_msgbuf mmb;
    int rcv_bytes = msgrcv(msqid2, &mmb, sizeof(struct message_msgbuf), 2, 0);

    while(rcv_bytes !=-1) {
        /*
        printf("Matcheng listening on msqi2d: %d\n", msqid2);
        */
        //Extract the OrderIDs for routing:
        switch (mmb.omm.type) {
            case NEW_ORDER_ACK:
                strncpy( order_id, mmb.omm.payload.orderAck.order_id, 32);
                break;
            case NEW_ORDER_NAK:
                strncpy( order_id, mmb.omm.payload.orderNak.order_id, 32);
                break;
            case CANCEL_ACK:
                strncpy( order_id, mmb.omm.payload.cancelAck.order_id, 32);
                break;
            case CANCEL_NAK:
                strncpy( order_id, mmb.omm.payload.cancelNak.order_id, 32);
                break;
            case MODIFY_ACK:
                strncpy( order_id, mmb.omm.payload.modifyAck.order_id, 32);
                break;
            case MODIFY_NAK:
                strncpy( order_id, mmb.omm.payload.modifyNak.order_id, 32);
                break;
            case NEW_ORDER:
            case MODIFY_REQ:
            case CANCEL_REQ:
                break;
            default:
                printf("**warning** this should never occur.\n");
        };

        writetoken.lock();
        //This was the original
        //strncpy( order_id, mmb.omm.payload.orderAck.order_id, 32);
        printf("======message received from matching enginee======\n"); 
        printf("received: %d bytes from matching engine\n", rcv_bytes);
        printf("receiver mmb type: %lu\n", mmb.mtype);
        printf("receiver omm type: %d\n", mmb.omm.type);
        printf("receiver omm timestamp: %llu\n", mmb.omm.payload.orderAck.timestamp);
        printf("receiver omm order_id: %s.\n", order_id); 
        std::string ordr(order_id);
        auto it = connectionmapper.find(order_id);
        if (it != connectionmapper.end()) {
            printf("descriptor matched for reverse routing: %d\n", it->second);
            send(it->second, &mmb.omm, sizeof(mmb.omm), 0);
            connectionmapper.erase(it);
            printf("removed map entry from connection manager.\n");
            found++;
        } else {
            printf("**warning: no file descripter matched for reverse routing**: %d\n", it->second);
            notfound++;
        }
        printf("======end message received from matching engine======\n"); 
        writetoken.unlock();
        //printOrderManagementMessage(&mmb.omm);
        std::this_thread::yield();
        rcv_bytes = msgrcv(msqid2, &mmb, sizeof(struct message_msgbuf), 2, 0);
    };
};

std::string isommvalid(struct OrderManagementMessage omm) {
    printf("Validating message of type: %d\n", omm.type);
    if (omm.type == 0 || omm.type == 3) { //If this is a new order or a modify order verify the quantity is non-negative.
        if (omm.payload.order.quantity < 0 ) {
            char order_id[33];
            strncpy(order_id, omm.payload.order.order_id, 33);
            order_id[32]='\0';
            std::string ordr(order_id);
            printf("Error!! Negative quantity detected for order: %s\n", order_id);
            std::string error ("Negative quantity");
            return error;
        } else {
            //Then there was no error
        }
    } 
    //printf("Quantity: %lu\n",  omm.payload.order.quantity );
    return std::string();
}

int main(){
    printf("starting connection manager...\n"); 

    //Trap the signal
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    int rc;

    /* shared memory initialization */
    key_t key_shm;
    key_shm = ftok(CMTOBPKEY1, 'b');
    struct OrderManagementMessage* shm;

    /* semaphore initialization */
    key_t semkey;
    
    struct sembuf sops;
    sops.sem_num =0;
    sops.sem_op = 1;
    sops.sem_flg =0;

    shmid=shmget(key_shm,sizeof(struct OrderManagementMessage),0666|IPC_CREAT);
    printf("shared memory segment id initialized to: %d\n", shmid);
    shmctl(shmid, IPC_RMID,NULL);
    if ((shmid = shmget(key_shm,sizeof(struct OrderManagementMessage),0666|IPC_CREAT)) < 0) {
       perror("shmget");
    }
    printf("shmid for shmat: %d\n", shmid);
    shm = (struct OrderManagementMessage*) shmat(shmid, NULL, 0);

    semkey = ftok(SEMKEY2,'b');
    if ( semkey == (key_t)-1 )
    {
        printf("main: ftok() for sem failed\n");
        return -1;
    };
    printf("semkey: %d\n", semkey); 
    semid = semget( semkey, NUMSEMS, 0666 | IPC_CREAT );
    semctl(semid,0,IPC_RMID,NULL);
    semid = semget( semkey, NUMSEMS, 0666 | IPC_CREAT );
    if ( semid == -1 )
    {
        printf("Error in semget(): %s\n", strerror(errno));
        return -1;
    }

    semop(semid,&sops,1);

    std::thread rfme(readfrommatchingengine);
    key_t key;
    key = ftok(CMTOMEKEY1, 'b');
    msqid = msgget(key, 0666 | IPC_CREAT);
    msgctl(msqid,IPC_RMID,NULL);
    msqid = msgget(key, 0666 | IPC_CREAT);
    printf("msgqid for cm to matching engine (msgqid) equal to: %d\n",msqid); 

    int hServerSocket;
    struct sockaddr_in Address;
    int nAddressSize=sizeof(struct sockaddr_in);
    struct pollfd fds[200];

    hServerSocket=socket(AF_INET,SOCK_STREAM,0);
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
    
    //Bind the socket
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR) {
        printf("\nCould not connect to host\n");
        return -1;
    }
    
    //Print socket information
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

    //Set the listen backlog
    rc = listen(hServerSocket, 32);
    if (rc < 0)
    {
        printf("error on listen.");
        exit(-1);
    }
    
    //Configure the initial listening socket
    fds[0].fd = hServerSocket;
    fds[0].events = POLLIN;
    
    int nfds=1;
    int timeout;
    timeout = (3 * 60 * 1000);
    int current_size;
    int new_sd;
    int end_server = FALSE;
    int i;
    int close_conn;
    int compress_array = FALSE;
    int j;
    do {
        printf("Waiting on poll\n");
        rc = poll(fds, nfds, timeout); 

        if (rc < 0) {
            printf("poll() failed\n");
            break;
        }
        
        //Check to see if the 3 minute time out expired
        if (rc == 0) {
            printf("poll() timed out\n");
            break;
        }
        
        //One ore more descriptors are readable, we need to determine which ones they are.
        current_size = nfds;
        for (i = 0; i < current_size; i++) {
            //Loop through to find the descriptors that returned
            //POLLIN and determine whether it's the listenin
            //or the active connection.

            if(fds[i].revents == 0)
                continue;
            
            //If revents is not POLLIN, it's an unexpected result
            //log and end the server.
            if(fds[i].revents != POLLIN) {
                printf("**error revents = %d\n**", fds[i].revents);
                end_server = TRUE;
                break;
            }
            
            if (fds[i].fd == hServerSocket) {
                printf("listening socket is readable\n");
                do {
                    //Accept all incoming connections that are queued up
                    //on the listening socket before we loop back and
                    //poll again
                    //printf("Running do loop.\n");
                    new_sd = accept(hServerSocket, NULL, NULL);
                    if (new_sd < 0) {
                        if (errno != EWOULDBLOCK) {
                            printf("accept () failed.");
                            end_server = TRUE;
                        
                        }
                        break;
                    }
                    printf("new: incoming connection on descripter %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } while (new_sd != -1);
            } else {
                printf("poll: descriptor %d is readable\n", fds[i].fd);
                close_conn = FALSE;
                struct OrderManagementMessage omm;
                rc = recv(fds[i].fd, &omm, sizeof(omm), 0);
                receivedfromtradebots++;
                std::string error = isommvalid(omm);
                char order_id[33];
                strncpy(order_id, omm.payload.order.order_id, 32);
                order_id[32] = '\0';
                printf("order_id char array pre: %s.\n", order_id);
                printf("time: %lld\n", (long long) time(NULL));

                char acct[33];
                strncpy(acct, omm.payload.order.account, 32);
                acct[32] = '\0';

                char usr[33];
                strncpy(usr, omm.payload.order.user, 32);
                usr[32] = '\0';
                std::string ordr(order_id);

                writetoken.lock();
                printf("======incoming order======\n");
                printf("read %d bytes through socket with file descripter %d, current size: %d\n",rc, fds[i].fd, current_size);
                printf("message type: %d\n",omm.type);
                printf("order type: %d\n",omm.payload.order.order_type);
                printf("buysell: %d\n",omm.payload.order.buysell);
                printf("quantity: %lu\n",  omm.payload.order.quantity );
                printf("timestamp: %llu\n", omm.payload.order.timestamp );
                printf("received on fd (i): %d\n", i);
                printf("map size after insert: %lu\n", connectionmapper.size());
                printf("account: %s.\n", acct);
                printf("user: %s.\n", usr);
                printf("======end incoming order======\n");
                writetoken.unlock();
                
                // error checking
                if ( error.empty() ) {
                    printf("the most recent incoming order was valid..\n");
                } else { 
                    struct OrderManagementMessage romm;
                    printf("This message is not valid. Sending a nack.\n");
                    if (omm.type == 3) { 
                        romm.type = MODIFY_NAK;
                        strncpy( romm.payload.modifyNak.reason , error.c_str(),64);
                        printf("Reason set to: %s\n", romm.payload.modifyNak.reason);
                        strncpy( romm.payload.modifyNak.order_id , order_id,32);
                        romm.payload.modifyNak.timestamp = (long long) time(NULL);
                        printf("Timestamp set to: %llu\n",romm.payload.modifyNak.timestamp);
                    } else if (omm.type ==0)  {
                        romm.type = NEW_ORDER_NAK; 
                        printf("Configuring this as an order_nak\n");
                        strncpy( romm.payload.orderNak.reason , error.c_str(),64);
                        printf("Reason set to: %s\n", romm.payload.modifyNak.reason);
                        strncpy( romm.payload.orderNak.order_id , order_id, 32);
                        romm.payload.orderNak.timestamp = (long long) time(NULL);
                        printf("Timestamp set to: %llu\n",romm.payload.orderNak.timestamp);
                    } else {
                        printf("Unknown order type failed validation\n");
                    }
                    rc = send(fds[i].fd, &romm, sizeof(romm), 0);
                    printf("=========printnack======\n");
                    printOrderManagementMessage(&romm);
                    printf("sent %d bytes through socket in NAK message\n", rc);
                };
                // end of error checking

                //Insert the entry in to the connection mapper.
                printf("inserting %d as file descriptor for order %s.\n", fds[i].fd, ordr.c_str());
                connectionmapper.insert(std::pair<std::string,int>(ordr ,fds[i].fd ));

                //Send the message to the MatchingEngine 
                struct message_msgbuf mmb = {2, omm};
                printf("sending message to queue with queue id: %d\n", msqid);
                msgsnd(msqid, &mmb, sizeof(struct OrderManagementMessage ), 0);
                printf("successfully sent message to queue.\n");

                /* that the shared memory segment is busy.                   */
                sops.sem_num = 0;
                sops.sem_op = -1;
                sops.sem_flg = 0;
                printf("calling semop with semid %d and blocking until desired condition can be reached.\n", semid); 
                rc = semop( semid, &sops, 1 );
                printf("semop completed.\n"); 
                if (rc == -1)
                {
                    printf("main: semop() failed\n");
                    return -1;
                }
                
                memcpy(shm,&omm,sizeof(omm));
                printf("successfully copied message to shared memory with order type %d\n", omm.type);
                printf("%d\n",copiedthroughsharedmemory++);
                sops.sem_num = 1;
                sops.sem_op = 1;
                printf("calling semop with semid %d and blocking until desired condition can be reached.\n", semid);
                rc = semop(semid,&sops,1);
                printf("semop successful\n");
                if (rc == -1)
                {
                    printf("main: semop() failed\n");
                    return -1;
                }
                
                if (close_conn) {
                    printf("Close conn received\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = TRUE;
                }
            }
        }
        if (compress_array)
        {
            compress_array = FALSE;
            for (i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for(j = i; j < nfds; j++)
                    {
                        fds[j].fd = fds[j+1].fd;
                    }
                    nfds--;
                }
            }
        }
    } while (end_server == FALSE);
}
