#include <iostream>
#include <stdio.h>
#include "messages.h"
#include <cstdlib>
#include <string>
#include <map>
#include "OrderList.h"
#include "OrderBook.h"
#include <ctime>
#include <sys/time.h>
#include <sys/msg.h>
#include <stddef.h>
#include <signal.h>
#include <sys/types.h>

using namespace std;

map<string,OrderBook> obs; //order books by symbol
map<string,string> symbs;// symbols by orderid

//This is used to send the message through a System V message queue
struct message_msgbuf {
  long mtype;  /* must be positive */
  struct OrderManagementMessage omm;
};

key_t key1,key2;
int msqid1,msqid2;
struct message_msgbuf mmb;

/*intHandler closes queue and exits*/
void intHandler(int dummy=0){
  // closing my queue
  msgctl(msqid2,IPC_RMID,NULL);
  exit(0);
};


int higher(Order orderA, Order orderB, enum SIDE bos){
  if(atof(orderA.price) > atof(orderB.price)){
    return (bos==BUY)? 1:0;
  }else if(atof(orderA.price) < atof(orderB.price)){
    return (bos==SELL)? 1:0;
  }else{
    return (orderA.timestamp < orderB.timestamp)? 1:0;
  };
};

/*int addtobook(Order neworder){
  sqlite3 *db;
  sqlite3_open("OrderBook",&db);
  neworder.quantity = 0;
  cout << sqlite3_exec(db,"select * from t1",NULL,NULL, NULL) << endl;
  sqlite3_close(db);
  return 0;
  };*/

void process(Order myorder)
{
  string symbol(myorder.symbol);
  string orderid(myorder.order_id);
  symbs[orderid] = symbol;
  if(obs.count(symbol) < 1)
  {
    OrderBook ob(myorder.symbol);
    obs[symbol] = ob;
  };
  if(obs[symbol].AddOrder(myorder))
  {
    struct OrderManagementMessage myomm;
    struct OrderAck myoa;
    strcpy(myoa.order_id,myorder.order_id);
    struct timeval now;
    gettimeofday(&now, NULL);
    myoa.timestamp = now.tv_sec * 1000000000 + now.tv_usec;
    myomm.type = NEW_ORDER_ACK;
    myomm.payload.orderAck = myoa;
    struct message_msgbuf mmb = {2,myomm};
    msgsnd(msqid2,&mmb,sizeof(struct OrderManagementMessage),0);
  }else{
    struct OrderManagementMessage myomm;
    struct OrderNak myona;
    strcpy(myona.order_id,myorder.order_id);
    strcpy(myona.reason,"unable to add to book\n");
    struct timeval now;
    gettimeofday(&now, NULL);
    myona.timestamp = now.tv_sec * 1000000000 + now.tv_usec;
    myomm.type = NEW_ORDER_NAK;
    myomm.payload.orderNak = myona;
    struct message_msgbuf mmb = {2,myomm};
    msgsnd(msqid2,&mmb,sizeof(struct OrderManagementMessage),0);
  };
};

void process(Modify mymodify)
{
  string orderid(mymodify.order_id);
  if(symbs.count(orderid) < 1)
  {
    struct OrderManagementMessage myomm;
    struct ModifyNak mymnak;
    strcpy(mymnak.order_id,mymodify.order_id);
    strcpy(mymnak.reason,"Order ID is invalid");
    struct timeval now;
    gettimeofday(&now, NULL);
    mymnak.timestamp = now.tv_sec * 1000000000 + now.tv_usec;
    myomm.type = MODIFY_NAK;
    myomm.payload.modifyNak = mymnak;
    struct message_msgbuf mmb = {2,myomm};
    msgsnd(msqid2,&mmb,sizeof(struct OrderManagementMessage),0);
  }else{
    string symbol = symbs[orderid];
    pair<int,Order> oldorder = obs[symbol].RemoveOrder(mymodify.order_id);
    if (!oldorder.first)
    {
      struct OrderManagementMessage myomm;
      struct ModifyNak mymnak;
      strcpy(mymnak.order_id,mymodify.order_id);
      strcpy(mymnak.reason,"Order is no longer in book");
      struct timeval now;
      gettimeofday(&now, NULL);
      mymnak.timestamp = now.tv_sec * 1000000000 + now.tv_usec;
      myomm.type = MODIFY_NAK;
      myomm.payload.modifyNak = mymnak;
      struct message_msgbuf mmb = {2,myomm};
      msgsnd(msqid2,&mmb,sizeof(struct OrderManagementMessage),0);
    }else{
      Order neworder = oldorder.second;
      neworder.quantity = mymodify.quantity;
      strcpy(neworder.price,mymodify.price);
      if(!obs[symbol].AddOrder(neworder))
      {
        obs[symbol].AddOrder(oldorder.second);
      struct OrderManagementMessage myomm;
      struct ModifyNak mymnak;
      strcpy(mymnak.order_id,mymodify.order_id);
      strcpy(mymnak.reason,"Unable to update order");
      struct timeval now;
      gettimeofday(&now, NULL);
      mymnak.timestamp = now.tv_sec * 1000000000 + now.tv_usec;
      myomm.type = MODIFY_NAK;
      myomm.payload.modifyNak = mymnak;
      struct message_msgbuf mmb = {2,myomm};
      msgsnd(msqid2,&mmb,sizeof(struct OrderManagementMessage),0);
      }else{
        struct OrderManagementMessage myomm;
        struct ModifyAck mymak;
        strcpy(mymak.order_id,mymodify.order_id);
        struct timeval now;
        gettimeofday(&now, NULL);
        mymak.timestamp = now.tv_sec * 1000000000 + now.tv_usec;
        mymak.quantity = mymodify.quantity;
        myomm.type = MODIFY_ACK;
        myomm.payload.modifyAck = mymak;
        struct message_msgbuf mmb = {2,myomm};
        msgsnd(msqid2,&mmb,sizeof(struct OrderManagementMessage),0);
      };
    };
  };
};

void process(Cancel mycancel)
{
  string orderid(mycancel.order_id);
  if(symbs.count(orderid) < 1)
  {
    struct OrderManagementMessage myomm;
    struct CancelNak mycnak;
    strcpy(mycnak.order_id,mycancel.order_id);
    strcpy(mycnak.reason,"Order ID is invalid");
    struct timeval now;
    gettimeofday(&now, NULL);
    mycnak.timestamp = now.tv_sec * 1000000000 + now.tv_usec;
    myomm.type = CANCEL_NAK;
    myomm.payload.cancelNak = mycnak;
    struct message_msgbuf mmb = {2,myomm};
    msgsnd(msqid2,&mmb,sizeof(struct OrderManagementMessage),0);
  }else{
    string symbol = symbs[orderid];
    pair<int,Order> oldorder = obs[symbol].RemoveOrder(mycancel.order_id);
    if (!oldorder.first)
    {
      struct OrderManagementMessage myomm;
      struct CancelNak mycnak;
      strcpy(mycnak.order_id,mycancel.order_id);
      strcpy(mycnak.reason,"Order is no longer in book");
      struct timeval now;
      gettimeofday(&now, NULL);
      mycnak.timestamp = now.tv_sec * 1000000000 + now.tv_usec;
      myomm.type = CANCEL_NAK;
      myomm.payload.cancelNak = mycnak;
      struct message_msgbuf mmb = {2,myomm};
      msgsnd(msqid2,&mmb,sizeof(struct OrderManagementMessage),0);
    }else{
        struct OrderManagementMessage myomm;
        struct CancelAck mycak;
        strcpy(mycak.order_id,mycancel.order_id);
        mycak.quantity = oldorder.second.quantity;
        myomm.type = CANCEL_ACK;
        myomm.payload.cancelAck = mycak;
        struct message_msgbuf mmb = {2,myomm};
        msgsnd(msqid2,&mmb,sizeof(struct OrderManagementMessage),0);
    };
  };
};

void process(OrderManagementMessage omm)
{
  if (omm.type == NEW_ORDER)
  {
    Order myorder = omm.payload.order;
    process(myorder);
  } else if(omm.type == MODIFY_REQ){
    Modify mymodify = omm.payload.modify;
    process(mymodify);
  } else if(omm.type == CANCEL_REQ){
    Cancel mycancel = omm.payload.cancel;
    process(mycancel);
  } else {
    printf("Message cannot be processed\n");
  };
};


int main(){
  // set up queues
  key1 = ftok("/etc/updatedb.conf", 'b');
  msqid1 = msgget(key1, 0666 | IPC_CREAT);
  key2 = ftok("/etc/usb_modeswitch.conf", 'b');
  msqid2 = msgget(key2, 0666 | IPC_CREAT);
  // load database
  // some testing
  Order orderA = {LIMIT_ORDER,"5647","Charlie","4X54",21345,BUY,"MSFT","11.3",500};
  Order orderB = {LIMIT_ORDER,"5314","Delta","450X",24520,BUY,"MSFT","11.4",540};
  printf("%d\n", higher(orderA,orderB,BUY));
  // setting up
  printf("starting matching engine\n");
  // reading from message queue
  signal(SIGINT,intHandler);
  for(;;) {
    msgrcv(msqid1, &mmb, sizeof(struct message_msgbuf), 2, 0);
    // printf("Order type: %d\n",mmb.omm.payload.order.order_type);
    // printf("Buysell: %d\n",mmb.omm.payload.order.buysell);
    process(mmb.omm);
  };
  return 0;
};
