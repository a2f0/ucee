#ifndef BOOKPUB_H
#define BOOKPUB_H

#include "printing.h"
#include "orderbookview.h"

//This is used to send the message through a System V message queue
struct message_msgbuf {
  long mtype;  /* must be positive */
  struct OrderManagementMessage omm;
};

// derived class from OrderBookView, implements communication to clients
class BookPubOBV : public OrderBookView
{
public:
  void CommunicateBookMsg(struct BookMessage);
};

void BookPubOBV::CommunicateBookMsg(struct BookMessage bk_msg){
  
ssize_t f = sendto(mysocket, &bk_msg, 32, 0, (struct sockaddr*) &grp, sizeof(grp));
if(f<0){
        //TO DO:: ERROR HANDLING
        //fprintf(stderr,"Message Not Sent.\nUsage: ./sn -a 239.192.07.07 -p 1234\n");
        //return -1;
        }
}
;


#endif
