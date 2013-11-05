#include "OrderList.h"

int main()
{
  cout << "Hello world!" << endl;
  Order orderA = {LIMIT_ORDER,"5147","Charlie","14X54",21345,BUY,"MSFT","11.3",500};
  Order orderB = {LIMIT_ORDER,"5214","Delta","2450X",24520,BUY,"MSFT","11.4",540};
  Order orderC = {LIMIT_ORDER,"5147","Charlie","34X54",21345,BUY,"MSFT","11.4",500};
  Order orderD = {MARKET_ORDER,"5214","Delta","4450X",24520,BUY,"MSFT","11.4",540};
  OrderList OL;
  strcpy(OL.pricelevel, "11.4");
  OL.type = LIMIT_ORDER;
  OL.AddOrder(orderA);
  OL.AddOrder(orderB);
  OL.AddOrder(orderC);
  OL.AddOrder(orderD);
  OL.RemoveOrder(orderA.order_id);
  OL.RemoveOrder(orderB.order_id);
  OL.RemoveOrder(orderC.order_id);
  OL.RemoveOrder(orderD.order_id);
  OL.RemoveOrder(orderB.order_id);
  return 0;
};
