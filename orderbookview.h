#ifndef ORDERBOOKVIEW_H
#define ORDERBOOKVIEW_H


#include <cstring>
#include <iostream>
#include "messages.h"
#include <list>
#include <cmath>
#include <cstdlib>
#include <utility>
#include <sys/msg.h>
#include <stddef.h>
using namespace std;

typedef struct Order Order;
typedef struct Modify Modify;
typedef struct Cancel Cancel;
typedef struct OrderAck OrderAck;
typedef struct ModifyAck ModifyAck;
typedef struct CancelAck CancelAck;
typedef struct OrderNak OrderNak;
typedef struct ModifyNak ModifyNak;
typedef struct CancelNak CancelNak;



class OrderList
{
public:
  enum ORDER_TYPE type;
  char pricelevel[PRICE_SIZE];
  list<Order> orders;
public:
  int AddOrder(Order);
  pair<int,Order> RemoveOrder(char*);
};

// add order to orderlist in correct order, assuming list is sorted
// checks that the order belongs to the list
// returns 1 if successful, 0 if unsuccessful
int OrderList::AddOrder(Order myorder)
{
  // error checks
  if (myorder.order_type != type)
  {
    cout << "Order added to list of different order type" << endl;
    return 0;
  };
  if(myorder.order_type == LIMIT_ORDER &&
     abs(atof(myorder.price)-atof(pricelevel)) > pow(.1,PRICE_SIZE))
  {
    cout << "Order added to list of different price level" << endl;
    return 0;
  };
  //main function
  unsigned long long ts = myorder.timestamp;
  list<Order>::iterator it = orders.begin();
  while( it != orders.end() && it->timestamp >= ts)
  {
    it++;
  };
  orders.insert(it, myorder);
  return 0;
};

// removes order from orderlist
// returns 1 for success, 0 for failure, together with the order removed
pair<int,Order> OrderList::RemoveOrder(char* myorderid)
{
  Order myorder;
  int result = 0;
  list<Order>::iterator it = orders.begin();
  while(it != orders.end())
  {
    // found order, get info about order to return
    if (strcmp (it->order_id,myorderid)== 0)
    {
      result = 1;
      myorder.order_type = it->order_type;
      strcpy(myorder.account,it->account);
      strcpy(myorder.user,it->user);
      strcpy(myorder.order_id,it->order_id);
      myorder.timestamp = it->timestamp;
      myorder.buysell= it->buysell;
      strcpy(myorder.symbol, it->symbol);
      strcpy(myorder.price, it->price);
      myorder.quantity = it->quantity;
      orders.erase(it); // remove order
      pair<int,Order> p(result,myorder);
      return p;
    };
    it++;
  };
  cout << "Order not found" << endl;
  pair<int,Order> p(result,myorder);
  return p;
};






class OrderBook
{
public:
  char instr_name[SYMBOL_SIZE]; // symbol name
  list<OrderList> sellbook;
  list<OrderList> buybook;
public:
  OrderBook(){};
  OrderBook(char* nm){strcpy(instr_name,nm);};
  int AddOrder(Order);
  pair<int,Order> RemoveOrder(char* orderid);
};

int OrderBook::AddOrder(Order myorder)
{
  //check
  if (strcmp(myorder.symbol,instr_name) !=0)
  {
    cout << "Adding order to wrong book" << endl;
    return 0;
  };
  list<OrderList>::iterator it;
  int adj = (myorder.buysell == BUY? 1:-1);
  if(myorder.buysell == BUY){
    it = buybook.begin();
  }else{
    it = sellbook.begin();
  };
  // place market orders at the front
  if (myorder.order_type == MARKET_ORDER)
  {
    if (it->type == MARKET_ORDER)
    {
      it->AddOrder(myorder);
    }else{
      OrderList OL;
      OL.type = MARKET_ORDER;
      if(myorder.buysell==BUY)
      {
        buybook.insert(it,OL);
      }else{
        sellbook.insert(it,OL);
      }
      it--;
      it->AddOrder(myorder);
    };
  };
  // look for appropriate space in order book
  if (myorder.order_type == LIMIT_ORDER)
  {
    while(it != buybook.end() && it!= sellbook.end()
          && adj*atof(it->pricelevel) > adj*atof(myorder.price))
      it++;
  };
  // insert in book
  it->AddOrder(myorder);
  return 1;
};

pair<int,Order> OrderBook::RemoveOrder(char* orderid)
{
  list<OrderList>::iterator it = buybook.begin();
  pair<int,Order> p;
  p.first = 0;
  while (it != buybook.end())
  {
    p = it->RemoveOrder(orderid);
    if (p.first==1){ return p;};
  };
  it = sellbook.begin();
  while (it != sellbook.end())
  {
    p = it->RemoveOrder(orderid);
    if(p.first==1){ return p;};
  };
  return p;
};





class OrderBookView
{
public:
  int msqid;  
  map<string,OrderBook> mybooks;
  map<string,Order> myorders;
public:
  OrderBookView(){};
  virtual void Communicate(enum MESSAGE_TYPE,char*,char*,unsigned long){};
  void Process(Order);
  void Process(Modify);
  void Process(Cancel);
  void Process(OrderManagementMessage);
};
  
void OrderBookView::Process(OrderManagementMessage omm)
{
  if (omm.type == NEW_ORDER)
  {
    Order myorder = omm.payload.order;
    Process(myorder);
  } else if(omm.type == MODIFY_REQ){
    Modify mymodify = omm.payload.modify;
    Process(mymodify);
  } else if(omm.type == CANCEL_REQ){
    Cancel mycancel = omm.payload.cancel;
    Process(mycancel);
  } else {
    printf("Message cannot be processed\n");
  };
};

void OrderBookView::Process(Order myorder)
{
  string symbol(myorder.symbol);
  string orderid(myorder.order_id);
  myorders[orderid] = myorder;
  if(mybooks.count(symbol) < 1)
  {
    OrderBook ob(myorder.symbol);
    mybooks[symbol] = ob;
  };
  if(mybooks[symbol].AddOrder(myorder))
  {
    Communicate(NEW_ORDER_ACK,myorder.order_id,NULL,0);
    // communicating OrderAck
  }else{
    char reason[REASON_SIZE];
    strcpy(reason, "unable to add book");
    Communicate(NEW_ORDER_NAK,myorder.order_id,reason,0);
    // communicating OrderNak
  };
};

void OrderBookView::Process(Cancel mycancel)
{
  string orderid(mycancel.order_id);
  if(myorders.count(orderid) < 1)
  {
    char reason[REASON_SIZE];
    strcpy(reason, "order id is invalid");
    Communicate(CANCEL_NAK,mycancel.order_id,reason,0);
  }else{
    Order myorder = myorders[orderid];
    string symbol(myorder.symbol);
    pair<int,Order> oldorder = mybooks[symbol].RemoveOrder(mycancel.order_id);
    if (!oldorder.first)
    {
      char reason[REASON_SIZE];
      strcpy(reason, "order is no longer in book");
      Communicate(CANCEL_NAK,mycancel.order_id,reason,0);
    }else{
      Communicate(CANCEL_ACK,mycancel.order_id,NULL,oldorder.second.quantity);
    };
  };
};

void OrderBookView::Process(Modify mymodify)
{
  string orderid(mymodify.order_id);
  if(myorders.count(orderid) < 1)
  {
    char reason[REASON_SIZE];
    strcpy(reason, "order id is invalid");
    Communicate(MODIFY_ACK,mymodify.order_id,reason,0);
  }else{
    Order myorder = myorders[orderid];
    string symbol(myorder.symbol);
    pair<int,Order> oldorder = mybooks[symbol].RemoveOrder(mymodify.order_id);
    if (!oldorder.first)
    {
      char reason[REASON_SIZE];
      strcpy(reason, "order is no longer in book");
      Communicate(MODIFY_NAK,mymodify.order_id,reason,0);
    }else{
      Order neworder = oldorder.second;
      neworder.quantity = mymodify.quantity;
      strcpy(neworder.price,mymodify.price);
      if(!mybooks[symbol].AddOrder(neworder))
      {
        mybooks[symbol].AddOrder(oldorder.second);
        char reason[REASON_SIZE];
        strcpy(reason, "unable to update order");
        Communicate(MODIFY_NAK,mymodify.order_id,reason,0);
      }else{
        Communicate(MODIFY_ACK,mymodify.order_id,NULL,mymodify.quantity);
      };
    };
  };
};


#endif
