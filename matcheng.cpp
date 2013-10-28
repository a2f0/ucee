#include <stdio.h>
#include "messages.h"
#include <cstdlib>

int higher(Order orderA, Order orderB, enum SIDE bos){
  if(atof(orderA.price) > atof(orderB.price)){
    return (bos==BUY)? 1:0;
  }else if(atof(orderA.price) < atof(orderB.price)){
    return (bos==SELL)? 1:0;
  }else{
    return (orderA.timestamp < orderB.timestamp)? 1:0;
  };
};

int main(){
  Order orderA = {LIMIT_ORDER,"5647","Charlie","4X54",21345,BUY,"MSFT","11.3",500};
  Order orderB = {LIMIT_ORDER,"5314","Delta","450X",24520,BUY,"MSFT","11.4",540};
  printf("%d\n", higher(orderA,orderB,BUY));
  printf("starting matching engine\n");
  return 0;
};
