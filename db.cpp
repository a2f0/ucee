/*
#include <stdio.h>
#include <iostream>
#include <string>
#include "sqlite3.h"
#include "messages.h"
using namespace std;
*/

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

if ( (rc = sqlite3_prepare_v2(mydb, "SELECT * FROM t1;",-1, &stmt3, NULL )) != SQLITE_OK)
        cout << endl << "Did you run ./database.sh?" << endl << sqlite3_errmsg(mydb) << endl;
cout<<"t1key|account|user|ordertype|timestamp|side|symbol|price|quantity"<<endl;
while ( (c=sqlite3_step(stmt3)) == 100 ){
        for(int j=0; j<8; j++)
                cout << sqlite3_column_text(stmt3,j) << "|";
	cout<<sqlite3_column_text(stmt3,8)<<endl;
}

sqlite3_finalize(stmt3);
sqlite3_close(mydb);
return 0;
}



int add_row(Order myorder){
int rc,c;
char* order_to_sql = (char*) malloc (1024*sizeof(char));
//sprintf(order_to_sql,"INSERT INTO t1 VALUES ('%s','%s','%s',%d,%llu,%d,'%s','%s',%lu);",nnstring(myorder.order_id, ORDERID_SIZE).c_str(),nnstring(myorder.account, ACCOUNT_SIZE).c_str(),nnstring(myorder.user,USER_SIZE).c_str(),(int)myorder.order_type,myorder.timestamp,myorder.buysell,nnstring(myorder.symbol,SYMBOL_SIZE).c_str(),nnstring(myorder.price,PRICE_SIZE).c_str(),myorder.quantity);
if(myorder.price==NULL){
	sprintf(myorder.price,"%s","0");
}
sprintf(order_to_sql,"INSERT INTO t1 VALUES ('%s','%s','%s',%d,%llu,%d,'%s','%s',%lu);",nnstring(myorder.order_id, ORDERID_SIZE).c_str(),nnstring(myorder.account, ACCOUNT_SIZE).c_str(),nnstring(myorder.user,USER_SIZE).c_str(),(int)myorder.order_type,myorder.timestamp,myorder.buysell,nnstring(myorder.symbol,SYMBOL_SIZE).c_str(),nnstring(myorder.price,PRICE_SIZE).c_str(),myorder.quantity);
//sprintf(order_to_sql,"INSERT INTO t1 VALUES ('%s','%s','%s',%d,%llu,%d,'%s',%f,%lu);",nnstring(myorder.order_id, ORDERID_SIZE).c_str(),myorder.account,myorder.user,(int)myorder.order_type,myorder.timestamp,myorder.buysell,myorder.symbol,atof(myorder.price),myorder.quantity);

sqlite3_stmt *stmt2;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);


if ( (rc = sqlite3_prepare_v2(mydb, order_to_sql,-1, &stmt2, NULL )) != SQLITE_OK)
//	throw string(sqlite3_errmsg(mydb));
	cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt2)) == 100 ){
	;
}
sqlite3_finalize(stmt2);
sqlite3_close(mydb);
return 0;
}

list<Order> get_db(std::string dbname, std::string tablename){
list<Order> mylist;
struct Order ooo;
int rc,c;
char* query1 = (char*) malloc (1024*sizeof(char));
sprintf(query1,"SELECT * FROM  %s", "t1");
sqlite3_stmt *stmt;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);
if ( (rc = sqlite3_prepare_v2(mydb, query1,-1, &stmt, NULL )) != SQLITE_OK)
//	throw string(sqlite3_errmsg(mydb));
	cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt)) == 100 ){
//	for(int j=0; j<8; j++) //there are 9 columns in the database as defined in database.sh


		//ooo.order_type = (ORDER_TYPE) atoi((const char*)sqlite3_column_text(stmt,3));		
		ooo.order_type = (ORDER_TYPE)/*(const char*)*/sqlite3_column_int(stmt,3);		
		//sprintf(ooo.account,"%s",sqlite3_column_text(stmt,1));		
		nstrcpy(ooo.account,(char*)sqlite3_column_text(stmt,1),ACCOUNT_SIZE);
		//sprintf(ooo.account,"%s",sqlite3_column_text(stmt,1));		
		nstrcpy(ooo.user,(char*)sqlite3_column_text(stmt,2),USER_SIZE);
		//sprintf(ooo.user,"%s",sqlite3_column_text(stmt,2));
		nstrcpy(ooo.order_id,(char*)sqlite3_column_text(stmt,0),ORDERID_SIZE);
		//sprintf(ooo.order_id,"%s",sqlite3_column_text(stmt,0));
//		ooo.timestamp=sqlite3_column_int(stmt,4);
		ooo.timestamp=strtoull((const char*)sqlite3_column_text(stmt,4),NULL,0);
		ooo.buysell=(SIDE) sqlite3_column_int(stmt,5);
		//ooo.buysell=(SIDE) atoi((const char*)sqlite3_column_text(stmt,5));
		nstrcpy(ooo.symbol,(char*)sqlite3_column_text(stmt,6),SYMBOL_SIZE);
		//sprintf(ooo.symbol,"%s",sqlite3_column_text(stmt,6));
		nstrcpy(ooo.price,(char*)sqlite3_column_text(stmt,7),PRICE_SIZE);
		//sprintf(ooo.price,"%s",sqlite3_column_text(stmt,7));
		//ooo.quantity=sqlite3_column_int(stmt,8);
		ooo.quantity=strtoul((const char*)sqlite3_column_text(stmt,8),NULL,0);
		printOrder(&ooo);
		mylist.push_back(ooo);
}

sqlite3_finalize(stmt);
sqlite3_close(mydb);
return mylist;
}

/*
int modify_row(Modify mymodify){
int rc,c;
char* query1 = (char*) malloc (1024*sizeof(char));
sprintf(query1,"UPDATE %s SET quantity='%lu', price='%f', timestamp='%llu' WHERE t1key='%s'", "t1", mymodify.quantity, atof(mymodify.price), mymodify.timestamp, mymodify.order_id);

sqlite3_stmt *stmt;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);

if ( (rc = sqlite3_prepare_v2(mydb, query1,-1, &stmt, NULL )) != SQLITE_OK)
//	throw string(sqlite3_errmsg(mydb));
	cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt)) == 100 ){
	;
}
sqlite3_finalize(stmt);
sqlite3_close(mydb);
return 0;
}
*/

int delete_row(char* order_id){
int rc,c;
char* order_to_sql = (char*) malloc (1024*sizeof(char));
sprintf(order_to_sql,"DELETE FROM t1 WHERE t1key='%s';",nnstring(order_id, ORDERID_SIZE).c_str());
//sprintf(order_to_sql,"DELETE FROM t1 WHERE t1key='%s';",order_id);
//nnstring(order_id, ORDERID_SIZE).c_str()

sqlite3_stmt *stmt2;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);

if ( (rc = sqlite3_prepare_v2(mydb, order_to_sql,-1, &stmt2, NULL )) != SQLITE_OK)
//      throw string(sqlite3_errmsg(mydb));
        cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt2)) == 100 ){
        ;
}
sqlite3_finalize(stmt2);
sqlite3_close(mydb);
return 0;
}
/*
int main2(){
//2147483647
Order orderA = {LIMIT_ORDER,"5647","Sarah","4X11xxxyxxx",21345,BUY,"GOOG","322.1",2147483649};
add_row(orderA);
Order orderAA = {LIMIT_ORDER,"5647","Sarah","4X11xyxxxx",21345,BUY,"GOOG","322.1",2147483645};
add_row(orderAA);
Order orderB = {LIMIT_ORDER,"5648","Jack","4X22",21345,BUY,"RAX","88.70",250};
add_row(orderB);
printf("\n\n\n");
printf("\n%s\n","1. Initial State of Table t1");
print_table();
printf("\n%s\n","2. Insert Row");
Order orderC = {LIMIT_ORDER,"5647       ","Charlie","4X99",21345,BUY,"MSFT","11.3",1000};
add_row(orderC);
print_table();
printf("\n%s\n","3. Modify Row From Step 2");
Modify modifyA = {"4X99",1111,"33.1",21545};
modify_row(modifyA);
print_table();
printf("\n%s\n","4. Delete Row From Step 2");
char* oid = (char*) malloc (sizeof(char));
sprintf(oid,"%s","4X99");
delete_row(oid);//takes order_id as parameter
print_table();
printf("\n%s\n","5. Pulling Database State to a list<Order>");
list<Order> mylist = list<Order>(get_db("OrderBook.db","t1"));
printf("\n%s\n","6. Printing that list<Order>");
for(std::list<Order>::const_iterator it = mylist.begin(); it != mylist.end(); ++it)
	printf("%s|%s|%s|%d|%llu|%d|%s|%f|%lu\n",it->order_id,it->account,it->user,(int)it->order_type,it->timestamp,it->buysell,it->symbol,atof(it->price),it->quantity);
printf("\n\n\n");

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

add_row(*a);

return 0;
}
*/

#endif
