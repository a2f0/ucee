#ifndef MATCHENG_H
#define MATCHENG_H

#include "printing.h"
#include "orderbookview.h"

// derived class from OrderBookView, implements communication to ConnMgr
class MatchEngOBV : public OrderBookView
{
public:
  void CommunicateTrade(struct TradeMessage);
  void CommunicateAck(enum MESSAGE_TYPE, char*, char*, unsigned long);
  void CommunicateReportingMsg(struct ReportingMessage);
};

// communicate trademessage to trade publisher
void MatchEngOBV::CommunicateTrade(struct TradeMessage tr_msg){
  noftrades++;
  struct sembuf sops;
  sops.sem_num = 0;
  sops.sem_op = -1;
  sops.sem_flg = 0;
  if(semop(semid,&sops,1)!=-1){
  struct TradeMessage* ptr = (struct TradeMessage*) shmat(shmid,NULL,0);
  nstrcpy(ptr->symbol,tr_msg.symbol,SYMBOL_SIZE);
  nstrcpy(ptr->price, tr_msg.price, PRICE_SIZE);
  ptr->quantity = tr_msg.quantity;
  sops.sem_num = 1;
  sops.sem_op = 1;
  semop(semid,&sops,1);
  // printTradeMsg(ptr);
  };
};

// communicate reportingmessage to reporting engine
void MatchEngOBV::CommunicateReportingMsg(struct ReportingMessage rp_msg){
  struct sembuf sops;
  sops.sem_num = 0;
  sops.sem_op = -1;
  sops.sem_flg = 0;
  if(semop(semidrp,&sops,1)!=-1){
  struct ReportingMessage* ptr=(struct ReportingMessage*)shmat(shmidrp,NULL,0);
  memcpy(ptr,&rp_msg,sizeof(rp_msg));
  sops.sem_num = 1;
  sops.sem_op = 1;
  // printReportingMsg(ptr);
  semop(semidrp,&sops,1);
  };
};

// communicate ack/nacks to connection manager
void MatchEngOBV::CommunicateAck(enum MESSAGE_TYPE type, char* id,
                                 char* reason, unsigned long quantity){
  nofackssent++;
  struct OrderManagementMessage myomm;
  struct timeval tmv;
  gettimeofday(&tmv,NULL);
  unsigned long long now = tmv.tv_sec * 1000000000 + tmv.tv_usec;
  myomm.type = type;
  switch (type) {
    case NEW_ORDER_ACK:
      struct OrderAck oack;
      nstrcpy(oack.order_id,id,ORDERID_SIZE);
      oack.timestamp = now;
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
  return;
};

#endif
