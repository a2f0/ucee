#define _BSD_SOURCE
#include <endian.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "messages.h"

/* Simple market data receiver */

int openSocket(void)
{
     int sock;

     sock = socket(AF_INET, SOCK_DGRAM, 0);
     if (sock < 0) {
	  perror("Opening datagram socket error");
	  return -1;
     }

     return sock;
}

int setReuseAddr(int sock)
{
     int reuse = 1;

     if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		    (char *)&reuse, sizeof(reuse)) < 0) {
	  perror("Error setting SO_REUSEADDR");
	  return -1;
     }

     return sock;
}

int bindSocket(int sock, int port)
{
     struct sockaddr_in sa;

     memset((char *) &sa, 0, sizeof(sa));
     sa.sin_family = AF_INET;
     sa.sin_port = htons(port);
     sa.sin_addr.s_addr = INADDR_ANY;
     if (bind(sock, (struct sockaddr*)&sa, sizeof(sa))) {
	  perror("Binding datagram socket error");
	  return -1;
     }

     return sock;
}

int joinMulticastGroup(int sock, const char *address)
{
     struct ip_mreq group;

     group.imr_multiaddr.s_addr = inet_addr(address);
     group.imr_interface.s_addr = INADDR_ANY;
     if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		    (char *)&group, sizeof(group)) < 0) {
	  perror("Error adding multicast group");
	  return -1;
     }

     return sock;
}

void printPriceLevel(struct BookData *data)
{
     char price[PRICE_SIZE + 1];

     memset(price, '\0', PRICE_SIZE + 1);
     memcpy(price, data->price, PRICE_SIZE);

     printf("%s: %lu", price, be64toh(data->quantity));
}

void processBookMessage(unsigned char *buffer)
{
     struct BookMessage *bookMessage = (struct BookMessage *)buffer;
     char symbol[SYMBOL_SIZE + 1];

     memset(symbol, '\0', SYMBOL_SIZE + 1);
     memcpy(symbol, bookMessage->symbol, SYMBOL_SIZE);
     printf("Received book message:\n  Book for %s:\n", symbol);
     for (int i = 0; i < 5; i++) {
	  printf("    Level %d | ", i + 1);
	  printPriceLevel(&bookMessage->bid[i]);
	  printf(" | ");
	  printPriceLevel(&bookMessage->offer[i]);
	  printf("\n");
     }
     printf("\n");
}

void processTradeMessage(unsigned char *buffer)
{
     struct TradeMessage *tradeMessage = (struct TradeMessage *)buffer;
     char symbol[SYMBOL_SIZE + 1];
     char price[PRICE_SIZE + 1];

     memset(symbol, '\0', SYMBOL_SIZE + 1);
     memcpy(symbol, tradeMessage->symbol, SYMBOL_SIZE);

     memset(price, '\0', PRICE_SIZE + 1);
     memcpy(price, tradeMessage->price, PRICE_SIZE);
     printf("Received trade message:\n");
     printf("  %s traded %lu at %s\n\n",
	    symbol, be64toh(tradeMessage->quantity), price);

}

int printMD(int sd)
{
     const unsigned int BufferSize = 9*1024;
     unsigned char buffer[BufferSize];

     while (1) {
	  ssize_t bytesRead = read(sd, buffer, BufferSize);

	  switch (bytesRead) {
	  case sizeof(struct BookMessage):
	       processBookMessage(buffer);
	       break;
	  case sizeof(struct TradeMessage):
	       processTradeMessage(buffer);
	       break;
	  default:
	       if (bytesRead < 0)
		    perror("Error reading market data message");
	       else
		    fprintf(stderr, "Unknown message received.\n");
	       return -1;
	  }
     }

     return 0;
}

int main(int argc, char **argv)
{
     int error = 0;
     int opt;
     int sock;

     char *address = 0;
     int port = 0;

     while ((opt = getopt(argc, argv, "a:p:i:")) != -1) {
	  switch (opt) {
	  case 'a':
	       address = strdup(optarg);
	       break;
	  case 'p':
	       port = atoi(optarg);
	       break;
	  default:
	       error = -1;
	  }
     }

     if (error || !address || !port) {
	  fprintf(stderr, "Usage: %s -a <address> -p <port>\n",
		  argv[0]);
	  exit(-1);
     }

     if ((sock = openSocket()) < 0)
	  exit(-1);

     if (setReuseAddr(sock) < 0) {
	  close(sock);
	  exit(-1);
     }

     if (bindSocket(sock, port) < 0) {
	  close(sock);
	  exit(-1);
     }

     if (joinMulticastGroup(sock, address) < 0) {
	  close(sock);
	  exit(-1);
     }

     fprintf(stderr, "Listening for market data:\n");

     return(printMD(sock));
}
