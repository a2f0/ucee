#ifndef MATCHENG_H
#define MATCHENG_H

#include "orderbookview.h"

//This is used to send the message through a System V message queue
struct message_msgbuf {
  long mtype;  /* must be positive */
  struct OrderManagementMessage omm;
};

class MatchEngOBV : public OrderBookView
{
public:
  MatchEngOBV(){};
  void Communicate(enum MESSAGE_TYPE, char*, char*, unsigned long);
};

void MatchEngOBV::Communicate(enum MESSAGE_TYPE type, char* id, char* reason, unsigned long quantity)
{
  struct OrderManagementMessage myomm;
  struct timeval tm;
  unsigned long long now = tm.tv_sec * 1000000000 + tm.tv_usec;
  myomm.type = type;
  switch (type) {
    case NEW_ORDER_ACK:
      struct OrderAck oack;
      oack.timestamp = now;
      strcpy(oack.order_id,id);
      myomm.payload.orderAck = oack;
      break;
    case NEW_ORDER_NAK:
      struct OrderNak onak;
      strcpy(onak.reason,reason);
      onak.timestamp = now;
      strcpy(onak.order_id,id);
      myomm.payload.orderNak = onak;
      break;
    case CANCEL_ACK:
      struct CancelAck cack;
      cack.quantity = quantity;
      strcpy(cack.order_id,id);
      myomm.payload.cancelAck = cack;
      break;
    case CANCEL_NAK:
      struct CancelNak cnak;
      strcpy(cnak.reason,reason);
      cnak.timestamp = now;
      strcpy(cnak.order_id,id);
      myomm.payload.cancelNak = cnak;
      break;
    case MODIFY_ACK:
      struct ModifyAck mack;
      mack.quantity = quantity;
      mack.timestamp = now;
      strcpy(mack.order_id,id);
      myomm.payload.modifyAck = mack;
      break;
    case MODIFY_NAK:
      struct ModifyNak mnak;
      strcpy(mnak.reason,reason);
      mnak.timestamp = now;
      strcpy(mnak.order_id,id);
      myomm.payload.modifyNak = mnak;
      break;
    case NEW_ORDER:
    case MODIFY_REQ:
    case CANCEL_REQ:
      break;
  };
  struct message_msgbuf mmb = {2,myomm};
  msgsnd(msqid,&mmb,sizeof(struct OrderManagementMessage),0);
};


#endif
