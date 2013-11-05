#ifndef ORDERBOOK_H
#define ORDERBOOK_H
#include "messages.h"

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
