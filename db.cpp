/*
#include <stdio.h>
#include <iostream>
#include <string>
#include "sqlite3.h"
#include "messages.h"
using namespace std;
*/
#include "db.h"

sqlite3* create_db(){
sqlite3 *db = (sqlite3*) malloc (1024*1000*10*sizeof(char));
return db;
}


int print_table(){
int rc,c;

sqlite3_stmt *stmt3;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);

/*
SELECT STATEMENT
*/
if ( (rc = sqlite3_prepare_v2(mydb, "SELECT * FROM t1;",-1, &stmt3, NULL )) != SQLITE_OK)
        throw string(sqlite3_errmsg(mydb));
while ( (c=sqlite3_step(stmt3)) == 100 ){
        for(int j=0; j<7; j++)
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
sprintf(order_to_sql,"INSERT INTO t1 VALUES ('%s','%s','%s',%d,%llu,%d,'%s',%f,%lu);",myorder.order_id,myorder.account,myorder.user,1,myorder.timestamp,myorder.buysell,myorder.symbol,atof(myorder.price),myorder.quantity);

//stmt represents a sql query
sqlite3_stmt *stmt2;
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
/*
if ( (rc = sqlite3_prepare_v2(mydb, "SELECT * FROM t1;",-1, &stmt3, NULL )) != SQLITE_OK)
	throw string(sqlite3_errmsg(mydb));
while ( (c=sqlite3_step(stmt3)) == 100 ){
	for(int j=0; j<8; j++)
		cout << sqlite3_column_text(stmt3,j) << "|";
}

sqlite3_finalize(stmt3);
*/

sqlite3_close(mydb);
return 0;
}

//
//Please note: this get_db() function is not finished yet
//
list<Order> get_db(std::string dbname, std::string tablename){
list<Order> mylist;
struct Order ooo;
int rc,c;
char* query1 = (char*) malloc (1024*sizeof(char));
sprintf(query1,"SELECT * FROM  %s", "t1");
// SET quantity='%lu', price='%f', timestamp='%llu' WHERE t1key='%s'", "t1", mymodify.quantity, atof(mymodify.price), mymodify.timestamp, mymodify.order_id);
sqlite3_stmt *stmt;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);
if ( (rc = sqlite3_prepare_v2(mydb, query1,/* "INSERT INTO t1 VALUES ('mykey','myaccount','myuser', 1, 1, 1, 'mysymbol', 1, 1);"*/-1, &stmt, NULL )) != SQLITE_OK)
//	throw string(sqlite3_errmsg(mydb));
	cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt)) == 100 ){
	for(int j=0; j<8; j++) //there are 9 columns in the database as defined in database.sh

//Next step:  convert this data
/*

		ooo.order_type=sqlite3_column_text(stmt,3);		
		ooo.account=sqlite3_column_text(stmt,1);
		ooo.user=sqlite3_column_text(stmt,2);
		ooo.order_id=sqlite3_column_text(stmt,0);
		ooo.timestamp=sqlite3_column_text(stmt,4);
		ooo.buysell=sqlite3_column_text(stmt,5);
		ooo.symbol=sqlite3_column_text(stmt,6);
		ooo.price=sqlite3_column_text(stmt,7);
		ooo.quantity=sqlite3_column_text(stmt,8);
*/
		mylist.push_back(ooo);
}

sqlite3_finalize(stmt);
return mylist;
}


int modify_row(Modify mymodify){
int rc,c;
//char* queries[3];
char* query1 = (char*) malloc (1024*sizeof(char));
//queries[1] = (char* query2 = (char*) malloc (1024*sizeof(char)));
//queries[2] = (char* query3 = (char*) malloc (1024*sizeof(char)));

//Setting sql statements
//sprintf(queries[0],"UPDATE %s","t1");
sprintf(query1,"UPDATE %s SET quantity='%lu', price='%f', timestamp='%llu' WHERE t1key='%s'", "t1", mymodify.quantity, atof(mymodify.price), mymodify.timestamp, mymodify.order_id);


//stmt represents a sql query
sqlite3_stmt *stmt;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);

/*
INSERT STATEMENT
*/

if ( (rc = sqlite3_prepare_v2(mydb, query1,/* "INSERT INTO t1 VALUES ('mykey','myaccount','myuser', 1, 1, 1, 'mysymbol', 1, 1);"*/-1, &stmt, NULL )) != SQLITE_OK)
//	throw string(sqlite3_errmsg(mydb));
	cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt)) == 100 ){
	;
}
sqlite3_finalize(stmt);


/*
SELECT STATEMENT
*/
/*
if ( (rc = sqlite3_prepare_v2(mydb, "SELECT * FROM t1;",-1, &stmt3, NULL )) != SQLITE_OK)
	throw string(sqlite3_errmsg(mydb));
while ( (c=sqlite3_step(stmt3)) == 100 ){
	for(int j=0; j<8; j++)
		cout << sqlite3_column_text(stmt3,j) << "|";
	cout << sqlite3_column_text(stmt3,8) << endl;
}
sqlite3_finalize(stmt3);
*/

sqlite3_close(mydb);
return 0;
}


int delete_row(char* order_id){
int rc,c;
char* order_to_sql = (char*) malloc (1024*sizeof(char));
sprintf(order_to_sql,"DELETE FROM t1 WHERE t1key='%s';",order_id);

//stmt represents a sql query
sqlite3_stmt *stmt2;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);

/*
INSERT STATEMENT
*/

if ( (rc = sqlite3_prepare_v2(mydb, order_to_sql,/* "INSERT INTO t1 VALUES ('mykey','myaccount','myuser', 1, 1, 1, 'mysymbol', 1, 1);"*/-1, &stmt2, NULL )) != SQLITE_OK)
//      throw string(sqlite3_errmsg(mydb));
        cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt2)) == 100 ){
        ;
}
sqlite3_finalize(stmt2);

/*
SELECT STATEMENT
*/
/*
if ( (rc = sqlite3_prepare_v2(mydb, "SELECT * FROM t1;",-1, &stmt3, NULL )) != SQLITE_OK)
        throw string(sqlite3_errmsg(mydb));
while ( (c=sqlite3_step(stmt3)) == 100 ){
        for(int j=0; j<8; j++)
                cout << sqlite3_column_text(stmt3,j) << "|";
}

sqlite3_finalize(stmt3);
*/

sqlite3_close(mydb);
return 0;
}



int main(){

printf("\n%s\n","1. Initial State of Table t1");
print_table();
printf("\n%s\n","2. Insert Row");
Order orderB = {LIMIT_ORDER,"5647","Charlie","4X99",21345,BUY,"MSFT","11.3",1000};
add_row(orderB);
print_table();
printf("\n%s\n","3. Modify Row From Step 2");
Modify modifyB = {"4X99",1111,"33.1",21545};
modify_row(modifyB);
print_table();
printf("\n%s\n","4. Delete Row From Step 2");
char* oid = (char*) malloc (sizeof(char));
sprintf(oid,"%s","4X99");
delete_row(oid);//takes order_id as parameter
print_table();



/*
Below shows creating an order struct 2 different ways, conversions to char[N] and explicit
*/
/*
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
*/
/*
Order orderB = {LIMIT_ORDER,"5647","Charlie","4X56",21345,BUY,"MSFT","11.3",1000};
add_row(orderB);
*/

return 0;
}
