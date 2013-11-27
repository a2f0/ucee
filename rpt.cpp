#include "db.cpp"


int trader_rpt(char* tr){
printf("TRADER\n");

//list<Order> mylist = list<Order>(get_db_rpt(tr));
list<Order> mylist = list<Order>(get_db_rpt(""));
for(std::list<Order>::const_iterator it = mylist.begin(); it != mylist.end(); ++it)
        printf("%s|%s|%s|%d|%llu|%d|%s|%f|%lu\n",it->order_id,it->account,it->user,(int)it->order_type,it->timestamp,it->buysell,it->symbol,atof(it->price),it->quantity);
printf("\n\n\n");


//list<Order> get_db("dbname", "tablename");
return 0;
}


int instrument_rpt(char* in){
printf("INSTRUMENT\n");
return 0;
}


int summary_rpt(){
printf("SUMMARY\n");
return 0;
}

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
printf("\n\nUsage:  ./rpt -a [Report Type as an Int: 1,2, or 3] -p [Parameter (ie, trader id or instrument type) ]\n\n");
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
