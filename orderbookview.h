#ifndef ORDERBOOKVIEW_H
#define ORDERBOOKVIEW_H

#include <map>
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
#include "db.cpp"
#include <sys/socket.h>
//#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


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

int writetodatabase;
//int readfromdatabase;

//there is one of these for each price level
class OrderList
{
public:
  enum ORDER_TYPE type; // order type
  double price; // price level
  list<Order> orders; // list of orders, older at the front
public:
  OrderList(){price = 0;}; // constructor
  int AddOrder(Order); // returns 1 for success, 0 for failure
  int RemoveOrder(Order); // returns 1 for success, 0 for failure
  unsigned long long Quantity();
  void Print(); // prints orderlist;
};

// add order to orderlist in correct order, assuming list is sorted
// checks that the order belongs to the list
// returns 1 if successful, 0 if unsuccessful
int OrderList::AddOrder(Order myorder)
{ // error checks
  if (myorder.order_type != type)
  {
    cout << "Order added to list of different order type" << endl;
    return 0;
  };
  if(myorder.order_type == LIMIT_ORDER &&
     abs(price-natof(myorder.price,PRICE_SIZE)) > .00001)
  {
    cout << "Order added to list of different price level" << endl;
    return 0;
  };
  //main function
  if (orders.empty())
  {
    orders.push_front(myorder);
    if (writetodatabase==1)
      add_row(myorder);
    return 1;
  };
  unsigned long long ts = myorder.timestamp;
  list<Order>::iterator it = orders.begin();
  while( it != orders.end() && it->timestamp <= ts){
    it++;
  };
  orders.insert(it, myorder);
  if(writetodatabase==1)
    add_row(myorder);
  return 1;
};

// removes order from orderlist
// returns 1 for success, 0 for failure, together with the order removed
int OrderList::RemoveOrder(Order myorder)
{
  if (orders.empty())
    return 0;
  list<Order>::iterator it = orders.begin();
  while(it != orders.end())
  {
    // found order?
    if (nstrcmp (it->order_id,myorder.order_id,ORDERID_SIZE)== 0)
    {
      orders.erase(it); // remove order
      if(writetodatabase==1)
        delete_row(myorder.order_id);
      return 1;
    };
    it++;
  };
  cout << "Order not found" << endl;
  return 0;
};

unsigned long long OrderList::Quantity(){
  list<Order>::iterator it;
  unsigned long long q =0;
  for (it = orders.begin(); it!=orders.end();it++)
    q += it->quantity;
  return q;
};

void OrderList::Print(){
  if(!orders.empty()){
    printf("\n***ORDER LIST***PRICE:%f***ORDER:",price);
    printOrderType(type);
    printf("***SIZE:%d\n",(int)orders.size());
    list<Order>::iterator it = orders.begin();
    while(it != orders.end()){
      Order myorder = (*it);
      printOrder(&myorder);
      ++it;
    };
  };
//  printf("finished this list");
};


// keeps orders for one symbol
class OrderBook
{
public:
  char instr[SYMBOL_SIZE]; // instrument name/ticket/symbol
  list<OrderList> sellbook; // book of sell orders
  list<OrderList> buybook; // book of buy orders
public:
  OrderBook(){}; // used for STL MAP functionality
  OrderBook(char* nm){nstrcpy(instr,nm,SYMBOL_SIZE);};
  int AddOrder(Order); // returns 0 for failure, 1 for success
  int RemoveOrder(Order); // return 0 for failure, 1 for success
  struct ReportingMessage  Match();
  struct BookMessage TopBook();
  void Print(); // prints the sellbook and buybook
};

int OrderBook::AddOrder(Order myorder)
{
  //testing
//  printf("* OrderBook: adding an order as follows:\n");
//  printf("\t\tOrder's symbol: ");
//  printFixedLengthString(myorder.symbol,SYMBOL_SIZE);
//  printf("-\n");
//  printf("\t\tBook's symbol: ");
//  printFixedLengthString(instr,SYMBOL_SIZE);
//  printf("-\n");
  //check
  if(nstrcmp(myorder.symbol,instr,SYMBOL_SIZE)!=0)
  {
    cout << "Adding order to wrong book" << endl;
    return 0;
  };
//  printf("processing order of SIDE: %d",myorder.buysell);
  int adj = (myorder.buysell == BUY? 1:-1);// simplifies dealing buy vs sell
  // add the OrderList if none exists
  if ((adj==1 && buybook.empty()) || (adj == -1 && sellbook.empty()))
  {
    OrderList OL;
    OL.type = myorder.order_type;
    OL.price = natof(myorder.price,PRICE_SIZE);
    if(adj==1)
    {
      buybook.push_front(OL);
//      printf("modified buybook");
    }else{
      sellbook.push_front(OL);
//      printf("modified sellbook");
    };
  };
  // main functionality
  list<OrderList>::iterator it;
  if(adj==1){
    it = buybook.begin();
  }else{
    it = sellbook.begin();
  };
  // place market orders at the front
  if (myorder.order_type == MARKET_ORDER)
  {
    if (it->type == MARKET_ORDER)
    {
      return it->AddOrder(myorder); // adds order to OrderList
    }else{
      OrderList OL;
      OL.type = MARKET_ORDER;
      if(adj==1)
      {
        buybook.insert(it,OL);
//        printf("modified buybook");
      }else{
        sellbook.insert(it,OL);
//        printf("modified sellbook");
      }
      it--; // move back to new OrderList
      return it->AddOrder(myorder);
    };
  };
  if (it->type == MARKET_ORDER)
    it++;
  // look for appropriate space in order book for limit orders
  while(it != buybook.end() && it != sellbook.end()
        && adj*(it->price)
        > adj*natof(myorder.price,PRICE_SIZE))
      it++;
  // insert in book
  if (it == buybook.end() || it == sellbook.end()
      || abs(it->price-natof(myorder.price,PRICE_SIZE)) > .00001){
    OrderList OL;
    OL.type = LIMIT_ORDER;
    OL.price = natof(myorder.price,PRICE_SIZE);
    if(adj==1)
    {
      buybook.insert(it,OL);
    }else{
      sellbook.insert(it,OL);
    };
    it--;
  };
  return it->AddOrder(myorder);
};

int OrderBook::RemoveOrder(Order myorder)
{
  int adj = (myorder.buysell == BUY? 1:-1);// simplifies dealing buy vs sell
  if((adj==1 && buybook.empty())|| (adj==-1 && sellbook.empty())){
    return 0;
  };
  list<OrderList>::iterator it;
  if(adj==1){
    it = buybook.begin();
    while (it != buybook.end()){
      if(it->RemoveOrder(myorder)==1){
        if(it->orders.size()==0){
          buybook.erase(it);
        };
        return 1;
      };
      it++;
    };
  }else{
    it = sellbook.begin();
    while (it != sellbook.end()){
      if(it->RemoveOrder(myorder)==1){
        if(it->orders.size()==0){
          buybook.erase(it);
        };
        return 1;
      };
      it++;
    };
  };
  return 0;
};

void OrderBook::Print()
{
  printf("\n***Book for:");
  printFixedLengthString(instr,SYMBOL_SIZE);
  printf("\n");
  list<OrderList>::iterator it;
  if (!buybook.empty()){
  printf("\n***BUY SIDE***\n");
  for (it = buybook.begin(); it != buybook.end(); it++)
    it->Print();
  };
  if (!sellbook.empty()){
  printf("\n***SELL SIZE***\n");
  for (it = sellbook.begin(); it!= sellbook.end(); it++)
    it->Print();
  };
//  printf("finished buy and sell side\n");
};

struct ReportingMessage OrderBook::Match()
{
  Order orderA;
  Order orderB;
  struct TradeMessage tr_msg;
  nstrcpy(tr_msg.symbol,instr,SYMBOL_SIZE);
  tr_msg.quantity = 0;
  struct ReportingMessage rp_msg = {tr_msg,orderA,orderB};
  if (sellbook.empty()){
    if (buybook.front().type == MARKET_ORDER){
      buybook.pop_front();
    };
    return rp_msg;
  };
  if (buybook.empty()){
    if (sellbook.front().type == MARKET_ORDER){
      sellbook.pop_front();
    };
    return rp_msg;
  };
  orderA = sellbook.front().orders.front();
  orderB = buybook.front().orders.front();
  if(sellbook.front().type==LIMIT_ORDER && buybook.front().type==LIMIT_ORDER
     && buybook.front().price < sellbook.front().price)
    return rp_msg;
  // in other cases, we must have a match:
  tr_msg.quantity = min(orderA.quantity,orderB.quantity);
  if(sellbook.front().type == MARKET_ORDER){
    nstrcpy(tr_msg.price,orderB.price,PRICE_SIZE);
  }else if(buybook.front().type == MARKET_ORDER){
    nstrcpy(tr_msg.price,orderA.price,PRICE_SIZE);
  }else if(buybook.front().price >= sellbook.front().price){
    if(orderA.timestamp < orderB.timestamp){
      nstrcpy(tr_msg.price,orderA.price,PRICE_SIZE);
    }else{
      nstrcpy(tr_msg.price,orderB.price,PRICE_SIZE);
    };
  };
  //update book
  RemoveOrder(orderA);
  RemoveOrder(orderB);
  orderA.quantity = orderA.quantity - tr_msg.quantity;
  orderB.quantity = orderB.quantity - tr_msg.quantity;
  if(orderA.quantity > 0)
    AddOrder(orderA);
  if(orderB.quantity > 0)
    AddOrder(orderB);
  rp_msg.trademsg = tr_msg;
  rp_msg.orderA = orderA;
  rp_msg.orderB = orderB;
  return rp_msg;
};

struct BookMessage OrderBook::TopBook(){
  struct BookMessage mymsg;
  nstrcpy(mymsg.symbol,instr,SYMBOL_SIZE);
  struct BookData bid[5];
  struct BookData offer[5];
  list<OrderList>::iterator it;
  it = buybook.begin();
  int i;
  for (i =0;i < 5 && it != buybook.end(); it++){
    nstrcpy(bid[i].price, it->orders.front().price, PRICE_SIZE);
    bid[i].quantity = it->Quantity();
    i++;
  };
  while(i<5){
    nstringcpy(bid[i].price,"",PRICE_SIZE);
    bid[i].quantity = 0;
    i++;
  };
  
  it = sellbook.begin();
  for (i =0;i < 5 && it != sellbook.end(); it++){
      nstrcpy(offer[i].price, it->orders.front().price, PRICE_SIZE);
      offer[i].quantity = it->Quantity();
      i++;
  };
  while(i<5){
    nstringcpy(offer[i].price,"",PRICE_SIZE);
    offer[i].quantity = 0;
    i++;
  };
  memcpy(mymsg.bid,bid,sizeof(bid));
  memcpy(mymsg.offer,offer,sizeof(offer));
  return mymsg;
};


// OrderBookView class: view of all books
class OrderBookView
{
public:
  int msqid; // message queue to write acks/nacks
  int shmid; // shared memory
  int semid;
  int shmidrp; // shared memory to write reporting messages
  int semidrp;
  int mysocket;
  struct sockaddr_in grp;
  map<string,OrderBook> mybooks; // books by instrument
  map<string,Order> myorders; // orders by orderid
public:
  
  void Process(OrderManagementMessage); // calls other Process
  void Process(Order); // processes Order omm
  void ProcessDB(Order); //process Order from database
  void Process(Modify); // processes Modify omm
  void Process(Cancel); // processes Cancel omm
  void Print(); // prints all books
  virtual void CommunicateAck(enum MESSAGE_TYPE,char*,char*,unsigned long){};
  // communicates acks and naks to message queue
  virtual void CommunicateTrade(struct TradeMessage){};
  // communicates trades to shared memory
  virtual void CommunicateBookMsg(struct BookMessage){};
  // broadcasts bookmessage via multicast
  virtual void CommunicateReportingMsg(struct ReportingMessage){};
};

void OrderBookView::Process(OrderManagementMessage omm)
{
//  printf("* myBooks: processing OrderManagementMessage\n");
  if (omm.type == NEW_ORDER){
    Process(omm.payload.order);
  } else if(omm.type == MODIFY_REQ){
    Process(omm.payload.modify);
  } else if(omm.type == CANCEL_REQ){
    Process(omm.payload.cancel);
  } else {
//    printf("Message cannot be processed\n");
  };
};

void OrderBookView::Process(Order myorder)
{
  printf("* myBooks: processing Order\n");
  string symbol = nstring(myorder.symbol,SYMBOL_SIZE);
  cout << "* myBooks: here is the symbol: " << symbol << "-\n";
  string orderid = nstring(myorder.order_id,ORDERID_SIZE);
  cout << "* myBooks: here is the order id: " << orderid << "-\n";
  if(mybooks.count(symbol) < 1)
  {
    cout << "* myBooks: no book with that symbol" << endl;
    OrderBook ob(myorder.symbol);
    mybooks[symbol] = ob;
    cout << "* myBooks: added book with that symbol" << endl;
  }else{
    cout<< "* myBooks: book already exists, so won't create new" << endl;
  };
  if(mybooks[symbol].AddOrder(myorder)==1)
  {
    cout << "* myBooks: added order to corresponding book" << endl;
    if (myorder.order_type = LIMIT_ORDER)
      myorders[orderid] = myorder;
    cout << "* myBooks: added order into myorders" << endl;
    if (writetodatabase == 1){
      CommunicateAck(NEW_ORDER_ACK,myorder.order_id,NULL,0);
    };
    // communicating OrderAck
  }else{
    cout << "* myBooks: didn't add order\n" << endl;
    if (writetodatabase==1){
    char reason[REASON_SIZE];
    nstringcpy(reason,"unable to add to book",REASON_SIZE);
    CommunicateAck(NEW_ORDER_NAK,myorder.order_id,reason,0);
    // communicating OrderNak
    };
  };
  struct BookMessage mybookmsg;
  struct ReportingMessage rp_msg = mybooks[symbol].Match();
  int matches = 0;
  while(rp_msg.trademsg.quantity >0){
    matches++;
    CommunicateTrade(rp_msg.trademsg);
    CommunicateReportingMsg(rp_msg);
    rp_msg = mybooks[symbol].Match();
  };
  cout << "Performed the matching algorithm." << endl;
  // generate bookmessage
  if (matches > 0 || myorder.order_type == LIMIT_ORDER){
    cout << "Compiling book message" << endl;
    mybookmsg = mybooks[symbol].TopBook();
    cout << "Compiled book message" << endl;
    printBookMsg(&mybookmsg);
    CommunicateBookMsg(mybookmsg);
  };
};


void OrderBookView::Process(Modify mymodify)
{
  string orderid = nstring(mymodify.order_id,ORDERID_SIZE);
  if(myorders.count(orderid) < 1)
  {
    char reason[REASON_SIZE];
    nstringcpy(reason, "order id is invalid",REASON_SIZE);
    CommunicateAck(MODIFY_ACK,mymodify.order_id,reason,0);
  }else{
    Order myorder = myorders[orderid];
    string symbol = nstring(myorder.symbol,SYMBOL_SIZE);
    if((mybooks[symbol]).RemoveOrder(myorder) != 1)
    {
      char reason[REASON_SIZE];
      nstringcpy(reason, "order is no longer in book",REASON_SIZE);
      CommunicateAck(MODIFY_NAK,mymodify.order_id,reason,0);
    }else{
      myorders.erase(orderid);
      myorder.quantity = mymodify.quantity;
      nstrcpy(myorder.price,mymodify.price,REASON_SIZE);
      if(mybooks[symbol].AddOrder(myorder) != 1)
      {
        char reason[REASON_SIZE];
        nstringcpy(reason, "order cancel, unable to update order",REASON_SIZE);
        CommunicateAck(MODIFY_NAK,mymodify.order_id,reason,0);
      }else{
        myorders[orderid] = myorder;
        CommunicateAck(MODIFY_ACK,mymodify.order_id,NULL,mymodify.quantity);
      };
    };
    struct BookMessage mybookmsg;
    struct ReportingMessage rp_msg = mybooks[symbol].Match();
    int matches = 0;
    while(rp_msg.trademsg.quantity >0){
      matches++;
      CommunicateTrade(rp_msg.trademsg);
      CommunicateReportingMsg(rp_msg);
      rp_msg = mybooks[symbol].Match();
    };
    cout << "Performed the matching algorithm." << endl;
  // generate bookmessage
    if (matches > 0 || myorder.order_type == LIMIT_ORDER){
      cout << "Compiling book message" << endl;
      mybookmsg = mybooks[symbol].TopBook();
      cout << "Compiled book message" << endl;
      printBookMsg(&mybookmsg);
      CommunicateBookMsg(mybookmsg);
    };
  };
};

void OrderBookView::Process(Cancel mycancel)
{
  string orderid = nstring(mycancel.order_id,ORDERID_SIZE);
  if(myorders.count(orderid) < 1)
  {
    char reason[REASON_SIZE];
    nstringcpy(reason, "order id is invalid",REASON_SIZE);
    CommunicateAck(CANCEL_NAK,mycancel.order_id,reason,0);
  }else{
    Order myorder = myorders[orderid];
    string symbol = nstring(myorder.symbol,SYMBOL_SIZE);
    if ((mybooks[symbol]).RemoveOrder(myorder) != 1)
    {
      char reason[REASON_SIZE];
      nstringcpy(reason, "order is no longer in book",REASON_SIZE);
      CommunicateAck(CANCEL_NAK,mycancel.order_id,reason,0);
    }else{
      unsigned long quantity = myorders[orderid].quantity;
      myorders.erase(orderid);
      CommunicateAck(CANCEL_ACK,mycancel.order_id,NULL,quantity);
    };
    struct BookMessage mybookmsg;
    struct ReportingMessage rp_msg = mybooks[symbol].Match();
    int matches = 0;
    while(rp_msg.trademsg.quantity >0){
      matches++;
      CommunicateTrade(rp_msg.trademsg);
      CommunicateReportingMsg(rp_msg);
      rp_msg = mybooks[symbol].Match();
    };
    cout << "Performed the matching algorithm." << endl;
  // generate bookmessage
    if (matches > 0 || myorder.order_type == LIMIT_ORDER){
      cout << "Compiling book message" << endl;
      mybookmsg = mybooks[symbol].TopBook();
      cout << "Compiled book message" << endl;
      printBookMsg(&mybookmsg);
      CommunicateBookMsg(mybookmsg);
    };
  };
};

void OrderBookView::Print()
{
  printf("\n*** Order Book View ***\n");
  map<string,OrderBook>::iterator it;
  for (it = mybooks.begin();it!=mybooks.end();++it)
  {
    (it->second).Print();
//    printf("finished this book\n");
  };
};


#endif
