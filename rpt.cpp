#include "db.cpp"


int trader_rpt(char* tr){
printf("\n\nTRADER REPORT:\n\n");
int rc,c;
//char* query1 = (char*) malloc (1024*sizeof(char));
//char* query2 = (char*) malloc (1024*sizeof(char));
char* query3 = (char*) malloc (1024*sizeof(char));
//sprintf(query1,"%s","ATTACH 'OrderBook.db' as odb;");
//sprintf(query2,"%s","ATTACH 'Reporting.db' as rdb;");
sprintf(query3,"%s","SELECT * FROM t3 b INNER JOIN t2 a ON (b.orderid1 = a.t1key OR b.orderid2 = a.t1key) ORDER BY a.account, a.user;");
//sprintf(query1,"%s","ATTACH 'OrderBook.db' as odb;");
//sprintf(query2,"%s","ATTACH 'Reporting.db' as rdb;");
//sprintf(query3,"%s","SELECT * FROM rdb.t1 b INNER JOIN odb.t2 a ON (b.orderid1 = a.t1key OR b.orderid2 = a.t1key) ORDER BY a.account, a.user;");
//sqlite3_stmt *stmt1;
//sqlite3_stmt *stmt2;
sqlite3_stmt *stmt3;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);
//if ( (rc = sqlite3_prepare_v2(mydb, query1,-1, &stmt1, NULL )) != SQLITE_OK)
//        cout << sqlite3_errmsg(mydb);
//while ( (c=sqlite3_step(stmt1)) == 100 ){
//;
//}
//sqlite3_finalize(stmt1);


//if ( (rc = sqlite3_prepare_v2(mydb, query2,-1, &stmt2, NULL )) != SQLITE_OK)
//        cout << sqlite3_errmsg(mydb);
//while ( (c=sqlite3_step(stmt2)) == 100 ){
//;
//}
//sqlite3_finalize(stmt2);


if ( (rc = sqlite3_prepare_v2(mydb, query3,-1, &stmt3, NULL )) != SQLITE_OK)
        cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt3)) == 100 ){
      for(int j=0; j<6; j++) //number of columns in the database as defined in database.sh
		printf("%s | ",(char*)sqlite3_column_text(stmt3,j));
      printf("\n");
}

sqlite3_finalize(stmt3);
sqlite3_close(mydb);
return 0;
}




//list<Order> mylist = list<Order>(get_db_rpt(tr));
/*
list<Order> mylist = list<Order>(get_db_rpt(""));
for(std::list<Order>::const_iterator it = mylist.begin(); it != mylist.end(); ++it)
        printf("%s|%s|%s|%d|%llu|%d|%s|%f|%lu\n",it->order_id,it->account,it->user,(int)it->order_type,it->timestamp,it->buysell,it->symbol,atof(it->price),it->quantity);
printf("\n\n\n");
*/

//list<Order> get_db("dbname", "tablename");
//return 0;
//}


int instrument_rpt(char* in){
printf("\n\nINSTRUMENT REPORT:\n\n");
int rc,c;
//char* query1 = (char*) malloc (1024*sizeof(char));
//char* query2 = (char*) malloc (1024*sizeof(char));
char* query3 = (char*) malloc (1024*sizeof(char));
//sprintf(query1,"%s","ATTACH 'OrderBook.db' as odb;");
//sprintf(query2,"%s","ATTACH 'Reporting.db' as rdb;");
sprintf(query3,"%s","SELECT * FROM t3 b INNER JOIN t2 a ON (b.orderid1 = a.t1key OR b.orderid2 = a.t1key) ORDER BY a.symbol;");
//sqlite3_stmt *stmt1;
//sqlite3_stmt *stmt2;
sqlite3_stmt *stmt3;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);
//if ( (rc = sqlite3_prepare_v2(mydb, query1,-1, &stmt1, NULL )) != SQLITE_OK)
//        cout << sqlite3_errmsg(mydb);
//while ( (c=sqlite3_step(stmt1)) == 100 ){
//;
//}
//sqlite3_finalize(stmt1);


//if ( (rc = sqlite3_prepare_v2(mydb, query2,-1, &stmt2, NULL )) != SQLITE_OK)
//        cout << sqlite3_errmsg(mydb);
//while ( (c=sqlite3_step(stmt2)) == 100 ){
//;
//}
//sqlite3_finalize(stmt2);


if ( (rc = sqlite3_prepare_v2(mydb, query3,-1, &stmt3, NULL )) != SQLITE_OK)
        cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt3)) == 100 ){
      for(int j=0; j<6; j++) //number of columns in the database as defined in database.sh
                printf("%s | ",(char*)sqlite3_column_text(stmt3,j));
      printf("\n");
}

sqlite3_finalize(stmt3);
sqlite3_close(mydb);
return 0;
}


int summary_rpt(){
printf("\n\nSUMMARY REPORT:\n\n");
int rc,c;
char* query1 = (char*) malloc (1024*sizeof(char));
char* query2 = (char*) malloc (1024*sizeof(char));
char* query3 = (char*) malloc (1024*sizeof(char));
char* query4 = (char*) malloc (1024*sizeof(char));
char* query5 = (char*) malloc (1024*sizeof(char));
char* query6 = (char*) malloc (1024*sizeof(char));
char* query7 = (char*) malloc (1024*sizeof(char));
sprintf(query1,"%s","ATTACH 'OrderBook.db' as odb;");
sprintf(query2,"%s","ATTACH 'Reporting.db' as rdb;");
sprintf(query3,"%s","SELECT COUNT(*) FROM odb.t2;");
sprintf(query4,"%s","SELECT COUNT(*) FROM rdb.t1;");
sprintf(query5,"%s","SELECT SUM(quantity) FROM odb.t2 WHERE (side='0');");
sprintf(query6,"%s","SELECT SUM(quantity) FROM odb.t2 WHERE (side='1');");
sprintf(query7,"%s","SELECT SUM(quantity) FROM odb.t2;");
sqlite3_stmt *stmt1;
sqlite3_stmt *stmt2;
sqlite3_stmt *stmt3;
sqlite3_stmt *stmt4;
sqlite3_stmt *stmt5;
sqlite3_stmt *stmt6;
sqlite3_stmt *stmt7;
sqlite3* mydb = create_db();
sqlite3_open("OrderBook.db",&mydb);
if ( (rc = sqlite3_prepare_v2(mydb, query1,-1, &stmt1, NULL )) != SQLITE_OK)
        cout << sqlite3_errmsg(mydb);
//while ( (c=sqlite3_step(stmt1)) == 100 ){
//; 
//}
sqlite3_finalize(stmt1);



if ( (rc = sqlite3_prepare_v2(mydb, query2,-1, &stmt2, NULL )) != SQLITE_OK)
        cout << sqlite3_errmsg(mydb);
//while ( (c=sqlite3_step(stmt1)) == 100 ){
//; 
//}
sqlite3_finalize(stmt2);

printf("%s\n: ","Total Number of Orders");
if ( (rc = sqlite3_prepare_v2(mydb, query3,-1, &stmt3, NULL )) != SQLITE_OK)
        cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt3)) == 100 ){
                printf("%s \n ",(char*)sqlite3_column_text(stmt3,0));
                printf("%s \n ",(char*)sqlite3_column_text(stmt3,1));
                printf("%s \n ",(char*)sqlite3_column_text(stmt3,2));
//; 
}
sqlite3_finalize(stmt3);

printf("%s\n: ","Total Number of Trades");
if ( (rc = sqlite3_prepare_v2(mydb, query4,-1, &stmt4, NULL )) != SQLITE_OK)
        cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt4)) == 100 ){
                printf("%s \n ",(char*)sqlite3_column_text(stmt4,0));
                printf("%s \n ",(char*)sqlite3_column_text(stmt4,1));
                printf("%s \n ",(char*)sqlite3_column_text(stmt4,2));
//; 
}
sqlite3_finalize(stmt4);

printf("%s\n: ","Side 0 Volume");
if ( (rc = sqlite3_prepare_v2(mydb, query5,-1, &stmt5, NULL )) != SQLITE_OK)
        cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt5)) == 100 ){
                printf("%s \n ",(char*)sqlite3_column_text(stmt5,0));
                printf("%s \n ",(char*)sqlite3_column_text(stmt5,1));
                printf("%s \n ",(char*)sqlite3_column_text(stmt5,2));
//; 
}
sqlite3_finalize(stmt5);


printf("%s\n: ","Side 1 Volume");
if ( (rc = sqlite3_prepare_v2(mydb, query6,-1, &stmt6, NULL )) != SQLITE_OK)
        cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt6)) == 100 ){
//;
                printf("%s \n ",(char*)sqlite3_column_text(stmt6,0));
                printf("%s \n ",(char*)sqlite3_column_text(stmt6,1));
                printf("%s \n ",(char*)sqlite3_column_text(stmt6,2));
}
sqlite3_finalize(stmt6);


printf("%s\n: ","Total Volume");
if ( (rc = sqlite3_prepare_v2(mydb, query7,-1, &stmt7, NULL )) != SQLITE_OK)
        cout << sqlite3_errmsg(mydb);
while ( (c=sqlite3_step(stmt7)) == 100 ){	
//      for(int j=0; j<6; j++) //number of columns in the database as defined in database.sh
                printf("%s \n ",(char*)sqlite3_column_text(stmt7,0));
                printf("%s \n ",(char*)sqlite3_column_text(stmt7,1));
                printf("%s \n ",(char*)sqlite3_column_text(stmt7,2));
//      printf("\n");
}

sqlite3_finalize(stmt7);
sqlite3_close(mydb);
return 0;
}





/*
int instrument_rpt(char* in){
printf("INSTRUMENT\n");
return 0;
}
*/
/*
int summary_rpt(){
printf("SUMMARY\n");
return 0;
}
*/
int main(int argc, char* argv[]){
//int main(){

int c;
char* arga = (char*) malloc (1000*sizeof(char));
char* argp = (char*) malloc (1000*sizeof(char));

while ( (c = getopt(argc, argv, "a:p:")) != -1 ){
        switch(c){
        case 'a':
                strcpy(arga,optarg);
        case 'p':
                strcpy(argp,optarg);

        }
}
if (argc==5){
//printf("%s",arga);
//printf("%s",argp);
} else {
printf("\n\nUsage:  ./rpt -a [Report Type as an Int: 1,2, or 3 (1=Trader, 2=Instrument, 3=Summary)] -p [Parameter (ie, trader id or instrument type) ]\n\n");
}

if(atoi(arga)==1){
char* mychars = (char*) malloc (150*sizeof(char));
sprintf(mychars,"t");
trader_rpt(mychars);
}
if(atoi(arga)==2){
char* mychars = (char*) malloc (150*sizeof(char));
sprintf(mychars,"i");
instrument_rpt(mychars);
}
if(atoi(arga)==3){
summary_rpt();
}

return 0;
}
