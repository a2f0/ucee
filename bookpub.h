#ifndef BOOKPUB_H
#define BOOKPUB_H

#include "printing.h"
#include "orderbookview.h"
#include <errno.h>


// derived class from OrderBookView, implements communication to clients
class BookPubOBV : public OrderBookView
{
public:
  void CommunicateBookMsg(struct BookMessage);
};

void BookPubOBV::CommunicateBookMsg(struct BookMessage bk_msg){
  for(int i = 0; i<5;i++){
    bk_msg.bid[i].quantity = htobe64(bk_msg.bid[i].quantity);
    bk_msg.offer[i].quantity = htobe64(bk_msg.offer[i].quantity);
  };
  ssize_t f=sendto(mysocket,&bk_msg,sizeof(struct BookMessage),0,(struct sockaddr*) &grp, sizeof(grp));
  if(f<0){
        printf("Message Not Sent.\n");
        printf("%s",strerror(errno));
  };
};

#endif
