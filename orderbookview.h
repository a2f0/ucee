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
#include "printing.h"
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
     abs(atof(myorder.price)-atof(pricelevel)) > .00001)
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
      nstrcpy(myorder.account,it->account,ACCOUNT_SIZE);
      nstrcpy(myorder.user,it->user,USER_SIZE);
      nstrcpy(myorder.order_id,it->order_id,ORDERID_SIZE);
      nstrcpy(myorder.symbol, it->symbol,SYMBOL_SIZE);
      nstrcpy(myorder.price, it->price,PRICE_SIZE);
      myorder.timestamp = it->timestamp;
      myorder.buysell= it->buysell;
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
  char instr[SYMBOL_SIZE]; // symbol name
  list<OrderList> sellbook;
  list<OrderList> buybook;
public:
  OrderBook(){};
  OrderBook(char* nm){nstrcpy(instr,nm,SYMBOL_SIZE);};
  int AddOrder(Order);
  pair<int,Order> RemoveOrder(char* orderid);
  void Print();
};

int OrderBook::AddOrder(Order myorder)
{
  //check
  printFixedLengthString(myorder.symbol,SYMBOL_SIZE);
  printf("end\n");
  printFixedLengthString(instr,SYMBOL_SIZE);
  printf("end\n");
  if(nstrcmp(myorder.symbol,instr,SYMBOL_SIZE))
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

void OrderBook::Print()
{
  printFixedLengthString(instr,SYMBOL_SIZE);
  printf(":\n");
  printf("***BUY SIDE***\n");
  printf("***SELL SIZE***\n");
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
  void Process(OrderManagementMessage);
  void Process(Order);
  void Process(Modify);
  void Process(Cancel);
  void Print();
};

void OrderBookView::Print()
{
  printf("*** Order Book View ***\n\n");
  map<string,OrderBook>::iterator it;
  for (it = mybooks.begin();it!=mybooks.end();++it)
  {
    (it->second).Print();
  };
};

void OrderBookView::Process(OrderManagementMessage omm)
{
  printf("processing OrderManagementMessage\n");
  if (omm.type == NEW_ORDER)
  {
    Process(omm.payload.order);
  } else if(omm.type == MODIFY_REQ){
    Process(omm.payload.modify);
  } else if(omm.type == CANCEL_REQ){
    Process(omm.payload.cancel);
  } else {
    printf("Message cannot be processed\n");
  };
};

void OrderBookView::Process(Order myorder)
{
  printf("processing order\n");
  string symbol = nstring(myorder.symbol,SYMBOL_SIZE);
  cout << "symbol: " << symbol << "end\n";
  string orderid = nstring(myorder.order_id,ORDERID_SIZE);
  cout << "orderid: " << orderid << "end\n";
  myorders[orderid] = myorder;
  cout << "added order into myorders" << endl;
  if(mybooks.count(symbol) < 1)
  {
    cout << "no book with that symbol" << endl;
    OrderBook ob(myorder.symbol);
    mybooks[symbol] = ob;
  }else{
    cout<< "book already exists with that symbol" << endl;
  };
  if(mybooks[symbol].AddOrder(myorder))
  {
    cout << "Added order\n" << endl;
    Communicate(NEW_ORDER_ACK,myorder.order_id,NULL,0);
    // communicating OrderAck
  }else{
    cout << "didn't add order\n" << endl;
    char reason[REASON_SIZE];
    nstringcpy(reason, "unable to add book",REASON_SIZE);
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
    nstringcpy(reason, "order id is invalid",REASON_SIZE);
    Communicate(CANCEL_NAK,mycancel.order_id,reason,0);
  }else{
    Order myorder = myorders[orderid];
    string symbol(myorder.symbol);
    pair<int,Order> oldorder = mybooks[symbol].RemoveOrder(mycancel.order_id);
    if (!oldorder.first)
    {
      char reason[REASON_SIZE];
      nstringcpy(reason, "order is no longer in book",REASON_SIZE);
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
    nstringcpy(reason, "order id is invalid",REASON_SIZE);
    Communicate(MODIFY_ACK,mymodify.order_id,reason,0);
  }else{
    Order myorder = myorders[orderid];
    string symbol(myorder.symbol);
    pair<int,Order> oldorder = mybooks[symbol].RemoveOrder(mymodify.order_id);
    if (!oldorder.first)
    {
      char reason[REASON_SIZE];
      nstringcpy(reason, "order is no longer in book",REASON_SIZE);
      Communicate(MODIFY_NAK,mymodify.order_id,reason,0);
    }else{
      Order neworder = oldorder.second;
      neworder.quantity = mymodify.quantity;
      nstrcpy(neworder.price,mymodify.price,REASON_SIZE);
      if(!mybooks[symbol].AddOrder(neworder))
      {
        mybooks[symbol].AddOrder(oldorder.second);
        char reason[REASON_SIZE];
        nstringcpy(reason, "unable to update order",REASON_SIZE);
        Communicate(MODIFY_NAK,mymodify.order_id,reason,0);
      }else{
        Communicate(MODIFY_ACK,mymodify.order_id,NULL,mymodify.quantity);
      };
    };
  };
};


#endif
