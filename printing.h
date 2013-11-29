#ifndef PRINTING_H
#define PRINTING_H
#include "messages.h"
#include <cstdlib>
#include <string>
#include <cstdio>
#include <cstring>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>

struct ReportingMessage
{
  unsigned long long timestamp;
  struct TradeMessage trademsg;
  struct Order orderA;
  struct Order orderB;
};



// functionality to deal with strings without null termination

// compares two strings by adding null at end
int nstrcmp(char *s1, const char *s2, unsigned int size)
{
  char *buffer1 = (char*) malloc(size + 1);
  memset(buffer1, '\0', size + 1);
  memcpy(buffer1, s1, size);
  char *buffer2 = (char*) malloc(size + 1);
  memset(buffer2, '\0', size + 1);
  memcpy(buffer2, s2, size);
  int result = strcmp(buffer1,buffer2);
  free(buffer1);
  free(buffer2);
  //  cout << "Result from comparison: "<< result << endl;
  return result;
};

/* copies a string into char array, placing ' ' if extra space*/
void nstringcpy(char* dest, std::string s, unsigned int size)
{
  const char* cstr = s.c_str();
  memset(dest,' ',size);
  memcpy(dest, cstr, (strlen(cstr) < size)? strlen(cstr):size);
};

/* copies char array into another, placing ' ' if extra space*/
void nstrcpy(char *dest, char*src,unsigned int size)
{
  memset(dest, ' ', size);
  memcpy(dest, src, (strlen(src) < size) ? strlen(src) : size);
};

/* makes a char array into a string, leaving out blanks/etc*/
std::string nstring(char *s, unsigned int size)
{
  char* buffer = (char*)malloc(size +1);
  memset(buffer, '\0',size+1);
  for (unsigned int i = 0; !iscntrl(s[i])&&!isblank(s[i])&&i < size; i++)
    buffer[i]=s[i];
  std::string result(buffer);
  free(buffer);
  return result;
}

/* makes a char array into a string, NOT leaving out blanks/etc*/
std::string nnstring(char *s, unsigned int size)
{
  char* buffer = (char*)malloc(size +1);
  memset(buffer, '\0',size+1);
  for (unsigned int i = 0; i < size; i++)
    buffer[i]=s[i];
  std::string result(buffer);
  free(buffer);
  return result;
};


/* make a char array into a double*/
double natof(char* s, unsigned int size)
{
  char * buffer = (char*) malloc(size+1);
  memset(buffer,'\0',size+1);
  for (unsigned int i = 0; !iscntrl(s[i])&&!isblank(s[i])&&i < size; i++)
    buffer[i]=s[i];
  double result = atof(buffer);
  free(buffer);
  return result;
};


// functionality for printing

void printCurrentTime(void)
{
  struct timeval tv;
  struct tm time_tm;
  char buffer[256];

  gettimeofday(&tv, NULL);

  localtime_r(&tv.tv_sec, &time_tm);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %T", &time_tm);
  printf("\n[%s.%06lu000]", buffer, tv.tv_usec);
}

void printOrderType(enum ORDER_TYPE orderType)
{
  switch (orderType) {
    case MARKET_ORDER:
      printf("Market Order");
      break;
    case LIMIT_ORDER:
      printf("Limit Order");
      break;
    default:
      printf("Unknown order type");
  }
}

void printFixedLengthString(const char *s, unsigned int size)
{
  char *buffer = (char*) malloc(size + 1);
  memset(buffer, '\0', size + 1);
  memcpy(buffer, s, size);
  printf("%s", buffer);
  free(buffer);
}

void printTimestamp(unsigned long long timestamp)
{
  long s = timestamp / 1000000000;
  unsigned long us = timestamp % 1000000;
  struct tm time_tm;
  char buffer[256];

  localtime_r(&s, &time_tm);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %T", &time_tm);
  printf("%s.%06lu000", buffer, us);
}

void printSide(enum SIDE side)
{
  switch (side) {
    case BUY:
      printf("Buy");
      break;
    case SELL:
      printf("Sell");
      break;
    default:
      printf("Unknown side value");
  }
}

void printOrder(const struct Order *order)
{
  printCurrentTime();
  printf(" Order:");
  printf("\n  Order type: ");
  printOrderType(order->order_type);
  printf("\n  Account: ");
  printFixedLengthString(order->account, ACCOUNT_SIZE);
  printf("\n  User: ");
  printFixedLengthString(order->user, USER_SIZE);
  printf("\n  Order Id: ");
  printFixedLengthString(order->order_id, ORDERID_SIZE);
  printf("\n  Timestamp: ");
  printTimestamp(order->timestamp);
  printf("\n  Side: ");
  printSide(order->buysell);
  printf("\n  Instrument: ");
  printFixedLengthString(order->symbol, SYMBOL_SIZE);
  printf("\n  Price: ");
  if (order->order_type == MARKET_ORDER)
    printf("Market");
  else
    printFixedLengthString(order->price, PRICE_SIZE);
  printf("\n  Quantity: %lu\n", order->quantity);
}

void printOrderAck(const struct OrderAck *orderAck)
{
  printCurrentTime();
  printf(" OrderAck:");
  printf("\n  Order Id: ");
  printFixedLengthString(orderAck->order_id, ORDERID_SIZE);
  printf("\n  Timestamp: ");
  printTimestamp(orderAck->timestamp);
  printf("\n");
}

void printOrderNak(const struct OrderNak *orderNak)
{
  printCurrentTime();
  printf(" OrderNak:");
  printf("\n  Order Id: ");
  printFixedLengthString(orderNak->order_id, ORDERID_SIZE);
  printf("\n  Timestamp: ");
  printTimestamp(orderNak->timestamp);
  printf("\n  Failure reason: ");
  printFixedLengthString(orderNak->reason, REASON_SIZE);
  printf("\n");
}

void printModify(const struct Modify *modify)
{
  printCurrentTime();
  printf(" Modify:");
  printf("\n  Order Id: ");
  printFixedLengthString(modify->order_id, ORDERID_SIZE);
  printf("\n  Quantity: %lu", modify->quantity);
  printf("\n  Price: ");
  printFixedLengthString(modify->price, PRICE_SIZE);
  printf("\n  Timestamp: ");
  printTimestamp(modify->timestamp);
  printf("\n");
}

void printModifyAck(const struct ModifyAck *modifyAck)
{
  printCurrentTime();
  printf(" ModifyAck:");
  printf("\n  Order Id: ");
  printFixedLengthString(modifyAck->order_id, ORDERID_SIZE);
  printf("\n  Timestamp: ");
  printTimestamp(modifyAck->timestamp);
  printf("\n  Quantity: %lu\n", modifyAck->quantity);
}

void printModifyNak(const struct ModifyNak *modifyNak)
{
  printCurrentTime();
  printf(" ModifyNak:");
  printf("\n  Order Id: ");
  printFixedLengthString(modifyNak->order_id, ORDERID_SIZE);
  printf("\n  Timestamp: ");
  printTimestamp(modifyNak->timestamp);
  printf("\n  Failure reason: ");
  printFixedLengthString(modifyNak->reason, REASON_SIZE);
  printf("\n");
}

void printCancel(const struct Cancel *cancel)
{
  printCurrentTime();
  printf(" Cancel:");
  printf("\n  Order Id: ");
  printFixedLengthString(cancel->order_id, ORDERID_SIZE);
  printf("\n  Timestamp: ");
  printTimestamp(cancel->timestamp);
  printf("\n");
}

void printCancelAck(const struct CancelAck *cancelAck)
{
  printCurrentTime();
  printf(" CancelAck:");
  printf("\n  Order Id: ");
  printFixedLengthString(cancelAck->order_id, ORDERID_SIZE);
  printf("\n  Quantity: %lu\n", cancelAck->quantity);
}

void printCancelNak(const struct CancelNak *cancelNak)
{
  printCurrentTime();
  printf(" CancelNak:");
  printf("\n  Order Id: ");
  printFixedLengthString(cancelNak->order_id, ORDERID_SIZE);
  printf("\n  Timestamp: ");
  printTimestamp(cancelNak->timestamp);
  printf("\n  Failure reason: ");
  printFixedLengthString(cancelNak->reason, REASON_SIZE);
  printf("\n");
}

void printOrderManagementMessage(const struct OrderManagementMessage *omm)
{
  switch (omm->type) {
    case NEW_ORDER:
      printOrder(&omm->payload.order);
      break;
    case NEW_ORDER_ACK:
      printOrderAck(&omm->payload.orderAck);
      break;
    case NEW_ORDER_NAK:
      printOrderNak(&omm->payload.orderNak);
      break;
    case MODIFY_REQ:
      printModify(&omm->payload.modify);
      break;
    case MODIFY_ACK:
      printModifyAck(&omm->payload.modifyAck);
      break;
    case MODIFY_NAK:
      printModifyNak(&omm->payload.modifyNak);
      break;
    case CANCEL_REQ:
      printCancel(&omm->payload.cancel);
      break;
    case CANCEL_ACK:
      printCancelAck(&omm->payload.cancelAck);
      break;
    case CANCEL_NAK:
      printCancelNak(&omm->payload.cancelNak);
      break;
    default:
      fprintf(stderr, "Unknown message type\n");
  }
}

void printTradeMsg(const struct TradeMessage *tr_msg)
{
  printCurrentTime();
  printf(" TradeMessage:");
  printf("\n  Symbol: ");
  printFixedLengthString(tr_msg->symbol, SYMBOL_SIZE);
  printf("\n  Price: ");
  printFixedLengthString(tr_msg->price,PRICE_SIZE);
  printf("\n  Quantity: %lu", tr_msg->quantity);
  printf("\n");
};

void printBookData(const struct BookData *bk_data) // print 5 bookdata messages
{
  for(int i=0; i<5; i++){
    printf("Price: ");
    printFixedLengthString(bk_data[i].price, PRICE_SIZE);
    printf("\t Quantity: %llu\n", bk_data[i].quantity);
  };
};

void printBookMsg(const struct BookMessage *bk_msg)
{
  printf(" BookMessage:");
  printf("\n Symbol: ");
  printFixedLengthString(bk_msg->symbol, SYMBOL_SIZE);
  printf("\n Sellside: \n");
  printBookData(bk_msg->offer);
  printf("\n Buyside: \n");
  printBookData(bk_msg->bid);
};

void printReportingMsg(const struct ReportingMessage *rp_msg)
{
  printf(" ReportingMessage: \n");
  printf(" * Timestamp: %llu \n", rp_msg->timestamp);
  printf(" * TradeMessage: \n");
  struct TradeMessage tr_msg = rp_msg->trademsg;
  printTradeMsg(&tr_msg);
  struct Order orderA = rp_msg->orderA;
  printf(" * Order A: \n");
  printOrder(&orderA);
  struct Order orderB = rp_msg->orderB;
  printf(" * Order B: \n");
  printOrder(&orderB);
};
  

#endif
