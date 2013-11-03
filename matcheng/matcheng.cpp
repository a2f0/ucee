#include <iostream>
#include <stdio.h>
#include "messages.h"
#include <cstdlib>
#include <sqlite3.h>
#include <map>
#include "Container.h"
using namespace std;

int higher(Order orderA, Order orderB, enum SIDE bos){
  if(atof(orderA.price) > atof(orderB.price)){
    return (bos==BUY)? 1:0;
  }else if(atof(orderA.price) < atof(orderB.price)){
    return (bos==SELL)? 1:0;
  }else{
    return (orderA.timestamp < orderB.timestamp)? 1:0;
  };
};

int addtobook(Order neworder){
  sqlite3 *db;
  sqlite3_open("OrderBook",&db);
  neworder.quantity = 0;
  cout << sqlite3_exec(db,"select * from t1",NULL,NULL, NULL) << endl;
  sqlite3_close(db);
  return 0;
};

int main(){
  map<string,Orderbook> obs;
  Order orderA = {LIMIT_ORDER,"5647","Charlie","4X54",21345,BUY,"MSFT","11.3",500};
  Order orderB = {LIMIT_ORDER,"5314","Delta","450X",24520,BUY,"MSFT","11.4",540};
  printf("%d\n", higher(orderA,orderB,BUY));
  printf("starting matching engine\n");
  addtobook(orderA);
  return 0;
};
