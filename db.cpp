#include <stdio.h>
#include <iostream>
#include <string>
#include "sqlite3.h" 
#include "messages.h"
using namespace std;

sqlite3* create_db(){
sqlite3 *db;
return db;
}



int add_row(Order myorder){
int rc,c;
char* order_to_sql = (char*) malloc (1024*sizeof(char));
sprintf(order_to_sql,"INSERT INTO t1 VALUES ('%s','%s','%s',%d,%llu,%d,'%s',%f,%lu);",myorder.order_id,myorder.account,myorder.user,1,myorder.timestamp,myorder.buysell,myorder.symbol,atof(myorder.price),myorder.quantity);

//stmt represents a sql query
sqlite3_stmt *stmt2,*stmt3;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);

/*
INSERT STATEMENT
*/

if ( (rc = sqlite3_prepare_v2(mydb, order_to_sql,/* "INSERT INTO t1 VALUES ('mykey','myaccount','myuser', 1, 1, 1, 'mysymbol', 1, 1);"*/-1, &stmt2, NULL )) != SQLITE_OK)
//	throw string(sqlite3_errmsg(mydb));
	cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt2)) == 100 ){
	;
}
sqlite3_finalize(stmt2);

/*
SELECT STATEMENT
*/

if ( (rc = sqlite3_prepare_v2(mydb, "SELECT * FROM t1;",-1, &stmt3, NULL )) != SQLITE_OK)
	throw string(sqlite3_errmsg(mydb));
while ( (c=sqlite3_step(stmt3)) == 100 ){
	for(int j=0; j<8; j++)
		cout << sqlite3_column_text(stmt3,j) << "|";
	cout << sqlite3_column_text(stmt3,8) << endl;
}
sqlite3_finalize(stmt3);


sqlite3_close(mydb);
return 0;
}


//Note:  Removed the main function so db.cpp could be compiled with other .cpp files

int main(){

/*
Below shows creating an order struct 2 different ways, the first is basically just playing around with conversions to char[N]
*/

Order *a = (Order*) malloc (100*sizeof(Order));
a->order_type=LIMIT_ORDER;
snprintf(a->account,32,"%d",5647);
snprintf(a->user,32,"%s","Charlie");
snprintf(a->order_id,32,"%s","4X54");
a->timestamp=21345;
a->buysell=BUY;
snprintf(a->symbol,16,"%s","MSFT");
snprintf(a->price,10,"%f",11.3);
a->quantity=500;

//add_row(*a);

//More straightforward

Order orderB = {LIMIT_ORDER,"5647","Charlie","4X56",21345,BUY,"MSFT","11.3",1000};
add_row(orderB);

return 0;
}


