#ifndef PRINTING_H
#define PRINTING_H

void copyStringField(char *dest, const char *src, unsigned int size)
{
  memset(dest, ' ', size);
  memcpy(dest, src, (strlen(src) < size) ? strlen(src) : size);
}


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
  char *buffer = malloc(size + 1);
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

#endif
