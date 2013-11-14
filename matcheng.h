#ifndef MATCHENG_H
#define MATCHENG_H

#include "printing.h"
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
      nstrcpy(oack.order_id,id,ORDERID_SIZE);
      myomm.payload.orderAck = oack;
      break;
    case NEW_ORDER_NAK:
      struct OrderNak onak;
      nstrcpy(onak.reason,reason,REASON_SIZE);
      onak.timestamp = now;
      nstrcpy(onak.order_id,id,ORDERID_SIZE);
      myomm.payload.orderNak = onak;
      break;
    case CANCEL_ACK:
      struct CancelAck cack;
      cack.quantity = quantity;
      nstrcpy(cack.order_id,id,ORDERID_SIZE);
      myomm.payload.cancelAck = cack;
      break;
    case CANCEL_NAK:
      struct CancelNak cnak;
      nstrcpy(cnak.reason,reason,REASON_SIZE);
      cnak.timestamp = now;
      nstrcpy(cnak.order_id,id,ORDERID_SIZE);
      myomm.payload.cancelNak = cnak;
      break;
    case MODIFY_ACK:
      struct ModifyAck mack;
      mack.quantity = quantity;
      mack.timestamp = now;
      nstrcpy(mack.order_id,id,ORDERID_SIZE);
      myomm.payload.modifyAck = mack;
      break;
    case MODIFY_NAK:
      struct ModifyNak mnak;
      nstrcpy(mnak.reason,reason,REASON_SIZE);
      mnak.timestamp = now;
      nstrcpy(mnak.order_id,id,ORDERID_SIZE);
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
