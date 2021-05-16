#ifndef DB_H
#define DB_H
#include <algorithm>
#include "db.h"
#include "printing.h"

using namespace std;

sqlite3* create_db(){
  sqlite3 *db = (sqlite3*) malloc (1024*1000*10*sizeof(char));
  return db;
}


int print_table(){
  int rc,c;

  sqlite3_stmt *stmt3;
  sqlite3* mydb = create_db();
  sqlite3_open("OrderBook.db",&mydb);
  //prepare query
  if ( (rc = sqlite3_prepare_v2(mydb, "SELECT * FROM t1;",-1, &stmt3, NULL ))
       != SQLITE_OK){
    cout << endl << "Did you run ./database.sh?" << endl;
    cout << sqlite3_errmsg(mydb) << endl;
    cout<<"t1key|account|user|ordertype|timestamp|side|symbol|price|quantity";
    cout<<endl;
  };
  while((c=sqlite3_step(stmt3)) == 100){//go thru columns returned by query
    for(int j=0; j<8; j++)
      cout << sqlite3_column_text(stmt3,j) << "|";
    cout<<sqlite3_column_text(stmt3,8)<<endl;
  };
  //finalize statement
  sqlite3_finalize(stmt3);
  sqlite3_close(mydb);
  return 0;
};

int add_row(Order myorder){
  int rc,c;
  char* order_to_sql = (char*) malloc (1024*sizeof(char));

  if(myorder.order_type==MARKET_ORDER){
    nstringcpy(myorder.price,"",PRICE_SIZE);
  };
  //writing query
  sprintf(order_to_sql,
          "INSERT INTO t1 VALUES ('%s','%s','%s',%d,%llu,%d,'%s','%s',%lu);",
          nnstring(myorder.order_id, ORDERID_SIZE).c_str(),
          nnstring(myorder.account, ACCOUNT_SIZE).c_str(),
          nnstring(myorder.user,USER_SIZE).c_str(),
          (int)myorder.order_type,myorder.timestamp,myorder.buysell,
          nnstring(myorder.symbol,SYMBOL_SIZE).c_str(),
          nnstring(myorder.price,PRICE_SIZE).c_str(),myorder.quantity);

    sqlite3_stmt *stmt2;
    sqlite3* mydb = create_db();
    sqlite3_open("OrderBook.db",&mydb);

    if((rc=sqlite3_prepare_v2(mydb,order_to_sql,-1,&stmt2,NULL))!=SQLITE_OK)
      cout << sqlite3_errmsg(mydb);
    while ( (c=sqlite3_step(stmt2)) == 100 ){;
    };
    sqlite3_finalize(stmt2);
    sqlite3_close(mydb);
    free(order_to_sql);
    return 0;
};


//this function provides a list containing all the orders in the order book
//(those that haven't been matched)
list<Order> get_db(std::string dbname, std::string tablename){
  list<Order> mylist;
  struct Order ooo;
  int rc,c;
  char* query1 = (char*) malloc (1024*sizeof(char));
  sprintf(query1,"SELECT * FROM  %s", "t1");
  sqlite3_stmt *stmt;
  sqlite3* mydb = create_db();
  sqlite3_open("OrderBook.db",&mydb);
  if ((rc=sqlite3_prepare_v2(mydb, query1,-1, &stmt, NULL )) != SQLITE_OK)
    cout << sqlite3_errmsg(mydb);
  while((c=sqlite3_step(stmt))== 100){ //goes thru the columns returned
		ooo.order_type = (ORDER_TYPE)sqlite3_column_int(stmt,3);
		nstrcpy(ooo.account,(char*)sqlite3_column_text(stmt,1),ACCOUNT_SIZE);
		nstrcpy(ooo.user,(char*)sqlite3_column_text(stmt,2),USER_SIZE);
		nstrcpy(ooo.order_id,(char*)sqlite3_column_text(stmt,0),ORDERID_SIZE);
		ooo.timestamp=strtoull((const char*)sqlite3_column_text(stmt,4),NULL,0);
		ooo.buysell=(SIDE) sqlite3_column_int(stmt,5);
		nstrcpy(ooo.symbol,(char*)sqlite3_column_text(stmt,6),SYMBOL_SIZE);
		nstrcpy(ooo.price,(char*)sqlite3_column_text(stmt,7),PRICE_SIZE);
		ooo.quantity=strtoul((const char*)sqlite3_column_text(stmt,8),NULL,0);
		printOrder(&ooo);
		mylist.push_back(ooo);
  };

  sqlite3_finalize(stmt);
  sqlite3_close(mydb);
  free(query1);
  return mylist;
};


int delete_row(char* order_id){
  int rc,c;
  char* order_to_sql = (char*) malloc (1024*sizeof(char));
  //writing query
  sprintf(order_to_sql,"DELETE FROM t1 WHERE t1key='%s';",
          nnstring(order_id, ORDERID_SIZE).c_str());

  sqlite3_stmt *stmt2;
  sqlite3* mydb = create_db();
  sqlite3_open("OrderBook.db",&mydb);

  if ((rc=sqlite3_prepare_v2(mydb,order_to_sql,-1,&stmt2,NULL))!=SQLITE_OK)
    cout << sqlite3_errmsg(mydb);
  while ( (c=sqlite3_step(stmt2)) == 100 ){
    ;
  };
  sqlite3_finalize(stmt2);
  sqlite3_close(mydb);
  free(order_to_sql);
  return 0;
};

#endif
