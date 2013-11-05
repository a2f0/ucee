#ifndef ORDERLIST_H
#define ORDERLIST_H

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
typedef struct ModifyAck ModifyAck;
typedef struct CancelAck CancelAck;
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



#endif
