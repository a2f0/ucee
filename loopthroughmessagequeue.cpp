#include <sys/msg.h>
#include <stddef.h>
#include "messages.h"
#include <iostream>

//THis is used to send the message through a System V message queue
//http://beej.us/guide/bgipc/output/html/multipage/mq.html
//Must be padded with an long to send through a message queue, kind of weird but documented
struct message_msgbuf {
    long mtype;  /* must be positive */
    struct OrderManagementMessage omm;
};

key_t key;
int msqid;
struct OrderManagementMessage omm;
struct message_msgbuf mmb;

int main(){
    //some random file and character to seed the key master.  see http://beej.us/guide/bgipc/output/html/multipage/mq.html for clarification.
    key = ftok("/etc/updatedb.conf", 'b');
    msqid = msgget(key, 0666 | IPC_CREAT);
    for(;;) { 
        msgrcv(msqid, &mmb, sizeof(struct message_msgbuf), 2, 0);
        printf("Order type: %d\n",mmb.omm.payload.order.order_type); 
        printf("Buysell: %d\n",mmb.omm.payload.order.buysell);
    }
}

