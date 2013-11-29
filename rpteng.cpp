/*
#include <stdio.h>
#include <iostream>
#include <string>
#include "sqlite3.h"
#include "messages.h"
using namespace std;
*/
#include <algorithm>
#include "db.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "messages.h"
#include <cstdlib>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "db.cpp"
#include "keys.h"
int mysocket;
int sem_id;
int shmid;
void intHandler(int dummy=0){
  // closing IPCs
  shmctl(shmid,IPC_RMID,NULL);
  semctl(sem_id,0,IPC_RMID,NULL);
  exit(0);
};

/*
struct ReportingMessage
{
  struct TradeMessage trademsg;
  struct Order orderA;
  struct Order orderB;
};
*/

int add_row(ReportingMessage myrm){
int rc,c;
char* order_to_sql = (char*) malloc (1024*sizeof(char));
sprintf(order_to_sql,"INSERT INTO t3 VALUES ('%s',%llu,%s,%lu,'%s','%s');",nnstring(myrm.trademsg.symbol, SYMBOL_SIZE).c_str(),myrm.timestamp,nnstring(myrm.trademsg.price,PRICE_SIZE).c_str(),myrm.trademsg.quantity,nnstring(myrm.orderA.order_id, ORDERID_SIZE).c_str(),nnstring(myrm.orderB.order_id, ORDERID_SIZE).c_str());


//TEST WITH INSERTING ON LIMITED COLUMNS//sprintf(order_to_sql,"INSERT INTO t3 VALUES ('%s',%llu);",nnstring(myrm.trademsg.symbol, SYMBOL_SIZE).c_str(),myrm.orderA.timestamp);


//sprintf(order_to_sql,"INSERT INTO t3 VALUES ('%s',%llu,%f,%lu,%s,%s);",nnstring(myrm.trademsg.symbol, SYMBOL_SIZE).c_str(),myrm.orderA.timestamp,atof(myrm.trademsg.price),myrm.trademsg.quantity,myrm.orderA.order_id,myrm.orderB.order_id);

//sprintf(order_to_sql,"INSERT INTO t1 VALUES ('%s','%s','%s',%d,%llu,%d,'%s',%s,%lu);",nnstring(myorder.order_id, ORDERID_SIZE).c_str(),nnstring(myorder.account, ACCOUNT_SIZE).c_str(),nnstring(myorder.user,USER_SIZE).c_str(),(int)myorder.order_type,myorder.timestamp,myorder.buysell,nnstring(myorder.symbol,SYMBOL_SIZE).c_str(),nnstring(myorder.price,PRICE_SIZE).c_str(),myorder.quantity);

//nnstring(myorder.order_id, ORDERID_SIZE).c_str()
//(symbol TEXT, timestamp UNSIGNED BIG INT, price DOUBLE, quantity UNSIGNED BIG INT, orderid1 TEXT, orderid2 TEXT)
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





int main(){


//SHM SETUP
//
//int shmid/*,shmid2*/;
key_t mykey/*5678, mykey2=5679*/;
mykey = ftok(METOREKEY1,'b');
//size_t mysize = 27;
size_t mysize = sizeof(struct ReportingMessage);
struct ReportingMessage* rm = (struct ReportingMessage*) malloc (sizeof(ReportingMessage));
if( (shmid = shmget(mykey, mysize, 0666 | IPC_CREAT)) < 0)
        cout << "Error: shmget" << endl;
if ((rm = (struct ReportingMessage*) shmat(shmid, NULL, 0)) == (struct ReportingMessage*) -1) {
        cout << "Error: shmat" << endl;
}


//

//SOCKET SETUP
//
mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
unsigned char mc_ttl = 1;
setsockopt(mysocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl));
//

//SEM SETUP:
//
struct sembuf sops;
sem_id = semget(ftok(METORESEM,'b'), 2, 0666 | IPC_CREAT );
if(sem_id == -1){
perror("segment failed: ");
exit(EXIT_FAILURE);
}
//

sops.sem_num = 1;
sops.sem_op = -1;
sops.sem_flg = 0;

signal(SIGINT,intHandler);

while(semop(sem_id, &sops, 1)!=-1){ //RESERVE SEMAPHORE
//if ((rm = (struct ReportingMessage*) shmat(shmid, NULL, 0)) == (struct ReportingMessage*) -1) {
//        cout << "Error: shmat" << endl;
//}

//add_row(*rm);
printReportingMsg(rm);
add_row(*rm);



sops.sem_num = 0;
sops.sem_op = 1;
sops.sem_flg = 0;
semop(sem_id, &sops, 1); //RELEASE SEMAPHORE
sops.sem_num=1;
sops.sem_op=-1;

};
/*
if( (shmid = shmget(mykey, mysize, 0666 | IPC_CREAT)) < 0)
        cout << "Error: shmget" << endl;

for(;;){

if ((rm = (struct ReportingMessage*) shmat(shmid, NULL, 0)) == (struct ReportingMessage*) -1) {
        cout << "Error: shmat" << endl;
}

add_row(*rm);

}
*/

return 0;
}




int main2(){

Order orderA = {LIMIT_ORDER,"5647","Sarah","4X11",21345,BUY,"GOOG","322.1",750};
add_row(orderA);
Order orderB = {LIMIT_ORDER,"5648","Jack","4X22",21345,BUY,"RAX","88.70",250};
add_row(orderB);
printf("\n\n\n");
printf("\n%s\n","1. Initial State of Table t1");
print_table();
printf("\n%s\n","2. Insert Row");
Order orderC = {LIMIT_ORDER,"5647","Charlie","4X99",21345,BUY,"MSFT","11.3",1000};
add_row(orderC);
print_table();
//printf("\n%s\n","3. Modify Row From Step 2");
//Modify modifyA = {"4X99",1111,"33.1",21545};
//modify_row(modifyA);
//print_table();
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

return 0;
}
