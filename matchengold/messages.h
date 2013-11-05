/* -*- C++ -*- */

#ifndef MESSAGES_H
#define MESSAGES_H

/* All text fields left-justified, space (0x32) filled */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SYMBOL_SIZE 16
#define PRICE_SIZE 10
#define ACCOUNT_SIZE 32
#define USER_SIZE 32
#define ORDERID_SIZE 32
#define REASON_SIZE 64

     /* Order management messages */

     enum MESSAGE_TYPE {
	  NEW_ORDER,
	  NEW_ORDER_ACK,
	  NEW_ORDER_NAK,
	  MODIFY_REQ,
	  MODIFY_ACK,
	  MODIFY_NAK,
	  CANCEL_REQ,
	  CANCEL_ACK,
	  CANCEL_NAK
     };

     enum ORDER_TYPE {
	  MARKET_ORDER,
	  LIMIT_ORDER
     };

     enum SIDE {
	  BUY,
	  SELL
     };

     /* New Order */

     struct Order
     {
	  enum ORDER_TYPE order_type;
	  char account[ACCOUNT_SIZE];   // the client name
	  char user[USER_SIZE];	        // the user name at the client who is placing the order
	  char order_id[ORDERID_SIZE];  // a unique ID identifying this order at the client
	  unsigned long long timestamp; // in ns
	  enum SIDE buysell;		// buy or sell?
	  char symbol[SYMBOL_SIZE];	// like MSFT etc.
	  char price[PRICE_SIZE];	// Includes decimal point
	  unsigned long quantity;
     };

     struct OrderAck
     {
	  char order_id[ORDERID_SIZE];
	  unsigned long long timestamp;
     };

     struct OrderNak
     {
	  char order_id[ORDERID_SIZE];
	  char reason[REASON_SIZE];
	  unsigned long long timestamp;
     };

     /* Modify */

     struct Modify
     {
	  char order_id[ORDERID_SIZE];
	  unsigned long quantity;
	  char price[PRICE_SIZE];
	  unsigned long long timestamp;
     };

     struct ModifyAck
     {
	  char order_id[ORDERID_SIZE];
	  unsigned long long timestamp;
	  unsigned long quantity;
     };

     struct ModifyNak
     {
	  char order_id[ORDERID_SIZE];
	  char reason[REASON_SIZE];
	  unsigned long long timestamp;
     };

     /* Cancel */

     struct Cancel
     {
	  char order_id[ORDERID_SIZE];
	  unsigned long long timestamp;
     };

     struct CancelAck
     {
	  char order_id[ORDERID_SIZE];
	  unsigned long quantity;
     };

     struct CancelNak
     {
	  char order_id[ORDERID_SIZE];
	  char reason[64];
	  unsigned long long timestamp;
     };

     struct OrderManagementMessage
     {
	  enum MESSAGE_TYPE type;

	  union Payload {
	       struct Order order;
	       struct OrderAck orderAck;
	       struct OrderNak orderNak;
	       struct Modify modify;
	       struct ModifyAck modifyAck;
	       struct ModifyNak modifyNak;
	       struct Cancel cancel;
	       struct CancelAck cancelAck;
	       struct CancelNak cancelNak;
	  } payload;
     };

     /* Book message */

     struct BookData
     {
	  char price[PRICE_SIZE];
	  unsigned long long quantity;
     };

     struct BookMessage
     {
	  char symbol[SYMBOL_SIZE];

	  struct BookData bid[5];
	  struct BookData offer[5];
     };

     /* Trade message */

     struct TradeMessage
     {
	  char symbol[SYMBOL_SIZE];
	  char price[PRICE_SIZE];
	  unsigned long quantity;
     };

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif
