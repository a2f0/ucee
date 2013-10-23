#define _BSD_SOURCE
#define _POSIX_SOURCE

#include <errno.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <netinet/in.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "messages.h"

/* Globals (yuck) - needed for signal handler since we can't pass a
 * parameter in to it.*/

int sock;
unsigned char buy = 0;
unsigned char sell = 0;
char *instrument = 0;
unsigned int count = 0;
unsigned char countSpecified = 0;
unsigned char rateSpecified = 0;

char *orderIdPrefix = 0;
unsigned int orderNumber = 0;

char *accounts[] = {
     "Mark",
     "Chip",
     "Sai",
     "Eugene",
     "Solomon",
     "Mohammed",
     "Anand",
     "Joe",
     "Kathy",
     "Mike"
};

void copyStringField(char *dest, const char *src, unsigned int size)
{
     memset(dest, ' ', size);
     memcpy(dest, src, (strlen(src) < size) ? strlen(src) : size);
}

int openSocket(const char *address, const char *port)
{
     int result = 0;
     int sock;
     struct addrinfo *connectInfo;

     result = getaddrinfo(address, port, NULL, &connectInfo);
     if (result) {
	  fprintf(stderr, "Unable to get specified host/port: %s\n",
		  gai_strerror(result));
	  return -1;
     }

     sock = socket(connectInfo->ai_family,
		   connectInfo->ai_socktype,
		   connectInfo->ai_protocol);
     if (sock < 0) {
	  freeaddrinfo(connectInfo);
	  perror("Opening datagram socket error");
	  return -1;
     }

     result = connect(sock,
		      connectInfo->ai_addr,
		      connectInfo->ai_addrlen);
     if (result == -1) {
	  freeaddrinfo(connectInfo);
	  perror("Unable to connect");
	  return -1;
     }

     return sock;
}

char *baseOrderId(void)
{
     char retval[256];

     struct sockaddr_in sa;
     socklen_t len = sizeof(struct sockaddr);

     getsockname(sock, (struct sockaddr *)&sa, &len);
     snprintf(retval, sizeof(retval),
	      "%08X:%08X:", htonl(sa.sin_addr.s_addr), getpid());

     return strdup(retval);
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

void sendOrder(void)
{
     struct OrderManagementMessage omm;
     struct Order *order = &omm.payload.order;
     struct timeval now;
     struct tm now_tm;
     char buffer[256];
     struct passwd *pwent;
     int result = 0;

     gettimeofday(&now, NULL);
     localtime_r(&now.tv_sec, &now_tm);

     pwent = getpwuid(getuid());

     omm.type = NEW_ORDER;
     order->order_type = (random() % 2) ? MARKET_ORDER : LIMIT_ORDER;
     copyStringField(order->account,
		     accounts[random() %
			      (sizeof(accounts) / sizeof(accounts[0]))],
		     ACCOUNT_SIZE);
     copyStringField(order->user, pwent->pw_name, USER_SIZE);

     snprintf(buffer, ORDERID_SIZE + 1, "%s%08X",
	      orderIdPrefix, orderNumber++);
     copyStringField(order->order_id, buffer, ORDERID_SIZE);

     order->timestamp = now.tv_sec * 1000000000 + now.tv_usec;
     order->buysell = buy ? BUY : SELL;
     copyStringField(order->symbol, instrument, SYMBOL_SIZE);
     if (order->order_type == LIMIT_ORDER) {
	  snprintf(buffer, PRICE_SIZE + 1, "%lu", 200 - (random() % 100));
	  copyStringField(order->price, buffer, PRICE_SIZE);
     }
     order->quantity = random() % 100;

     result = write(sock, &omm, sizeof(struct OrderManagementMessage));

     if (result == -1)
	  perror("Unable to send order");
     else if (result == sizeof(struct OrderManagementMessage))
	  printOrderManagementMessage(&omm);
     else
	  printf("Unable to send order; 0 bytes transmitted\n");
}

void sendOrders(int numberToSend)
{
     for (int i = 0; i < numberToSend; i++)
	  sendOrder();
}

void timeoutHandler(int signal)
{
     sendOrder();
}

void defineTimeoutHandler(void)
{
     struct sigaction sa;

     sa.sa_handler = timeoutHandler;
     sa.sa_flags = 0;
     sigemptyset(&sa.sa_mask);
     sigaction(SIGALRM, &sa, NULL);
}

void setTimer(int rate)
{
     struct itimerval itv;

     if (rate) {
	  itv.it_interval.tv_sec = 0;
	  itv.it_interval.tv_usec = 1000000 / rate;

	  itv.it_value.tv_sec = 0;
	  itv.it_value.tv_usec = 1000000 / rate;

	  setitimer(ITIMER_REAL, &itv, NULL);
     }
}

void sendFirstOrder(void)
{
     sendOrder();
}

void printMessages(void)
{
     int result = 0;
     char buffer[sizeof(struct OrderManagementMessage)];

     while (1) {
	  result = read(sock, buffer, sizeof(struct OrderManagementMessage));
	  if (result == sizeof(struct OrderManagementMessage))
	       printOrderManagementMessage((struct OrderManagementMessage *)&buffer);
	  else if ((result == -1) && (errno == EINTR))
	       continue;
	  else
	       fprintf(stderr,
		       "Error receiving message; read() call returned %u bytes\n",
		       result);
     }

}

int main(int argc, char **argv)
{
     int error = 0;
     int opt;

     char *address = 0;
     char *port = 0;
     unsigned int rate = 0;	/* Per second */

     while ((opt = getopt(argc, argv, "i:bsr:c:a:p:")) != -1) {
	  switch (opt) {
	  case 'b':
	       if (buy || sell)
		    error = -1;
	       else
		    buy = 1;
	       break;
	  case 's':
	       if (buy || sell)
		    error = -1;
	       else
		    sell = 1;
	       break;
	  case 'i':
	       instrument = strdup(optarg);
	       break;
	  case 'r':
	       rate = atoi(optarg);
	       rateSpecified = 1;
	       break;
	  case 'c':
	       count = atoi(optarg);
	       countSpecified = 1;
	       break;
	  case 'a':
	       address = strdup(optarg);
	       break;
	  case 'p':
	       port = strdup(optarg);
	       break;
	  default:
	       error = -1;
	  }
     }

     if (error || !address || !port || !instrument || (buy && sell)) {
	  fprintf(stderr,
		  "Usage: %s {-b|-s} -i <instrument> [-r <rate>] [-c <count>] -a <address> -p <port>\n",
		  argv[0]);
	  exit(-1);
     }

     if ((sock = openSocket(address, port)) < 0)
	  exit(-1);

     srandom(time(NULL));
     orderIdPrefix = baseOrderId();

     if (rate) {
	  defineTimeoutHandler();
	  setTimer(rate);
	  sendFirstOrder();
     } else
	  sendOrders(count);

     printMessages();

     return 0;
}
