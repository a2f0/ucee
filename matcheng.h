#ifndef MATCHENG_H
#define MATCHENG_H


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
      orders.erase(it);
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
  char instr_name[SYMBOL_SIZE];
  list<OrderList> buybook;
  list<OrderList> sellbook;
public:
  OrderBook(){};
  OrderBook(char* symb){strcpy(instr_name,symb);};
  int AddOrder(Order);
  pair<int,Order> RemoveOrder(char*);
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


#endif
