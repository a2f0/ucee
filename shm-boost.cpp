//to compile:  g++ -I/usr/include/boost shm-boost.cpp -o shm-boost -lpthread -lrt
#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <string>
#include <cstdlib> //std::system
#include "messages.h"

using namespace boost::interprocess;

//Define an STL compatible allocator of ints that allocates from the managed_shared_memory.
//This allocator will allow placing containers in the segment
typedef allocator<char, managed_shared_memory::segment_manager>  CharAllocator;

//Alias a vector that uses the previous STL-like allocator so that allocates
//its values from the segment
typedef basic_string<char, std::char_traits<char>, CharAllocator>  MyShmString;
typedef allocator<MyShmString, managed_shared_memory::segment_manager>  StringAllocator;
typedef vector<MyShmString, StringAllocator>  MyShmStringVector;

//Main function. For parent process argc == 1, for child process argc == 2
int main(int argc, char *argv[])
{
   if(argc == 1){ //Parent process
      //Remove shared memory on construction and destruction
      struct shm_remove
      {
         shm_remove() { shared_memory_object::remove("MySharedMemory"); }
         ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
      } remover;

      //Create a new segment with given name and size
      managed_shared_memory segment(create_only, "MySharedMemory", 65536);


      //Initialize shared memory STL-compatible allocator
      const CharAllocator alloc_char (segment.get_segment_manager());
      const StringAllocator alloc_str (segment.get_segment_manager());

	MyShmString mystring(alloc_char);

	// Convert Trade Message to String
	//
	struct TradeMessage *mytm = (struct TradeMessage*) malloc (sizeof(TradeMessage));
	sprintf(mytm->symbol, "%s", "MSFT");	
	sprintf(mytm->price, "%f", 100.1);
	mytm->quantity=100;
	
	char* mystr2 = (char*) malloc (50*sizeof(char));
	sprintf(mystr2,"%s,%s,%lu",mytm->symbol,mytm->price,mytm->quantity);
	mystring=mystr2;
	std::cout<<"Server pushes trade message stored as CSV string:"<<std::endl<<mystr2<<std::endl;
	//end convert trademessage to string


	// Convert String to Trade Message
	//
	struct TradeMessage *mytm2 = (struct TradeMessage*) malloc (sizeof(TradeMessage));
	sscanf(mystr2,"%[^,],%[^,],%lu",mytm2->symbol,mytm2->price,&(mytm2->quantity));
//	printf("Trade Message Converted from CSV String:\n%s|",mytm2->symbol);
//	printf("%s|",mytm2->price);
//	printf("%lu\n",mytm2->quantity);
	//mystring=(mytm2->symbol);		
	//end convert string to trade message


	MyShmStringVector myvector(alloc_str);
	myvector.insert(myvector.begin(), 10, mystring);

	MyShmStringVector *myshmvector = segment.construct<MyShmStringVector>("myshmvector")(alloc_str);
	myshmvector->insert(myshmvector->begin(), 10, mystring);


      //Launch child process
      std::string s(argv[0]); s += " child ";
      if(0 != std::system(s.c_str()))
         return 1;

      //Check child has destroyed the vector
      if(segment.find<MyShmStringVector>("myshmvector").first)
         return 1;
   }
   else{ //Child process
      //Open the managed segment
      managed_shared_memory segment(open_only, "MySharedMemory");

      //Find the vector using the c-string name
      MyShmStringVector *myvector = segment.find<MyShmStringVector>("myshmvector").first;

std::cout<<"Client Pulls Trade Message Stored As CSV String:"<<std::endl<<myvector->front()<<std::endl;

      //When done, destroy the vector from the segment
      segment.destroy<MyShmStringVector>("myshmvector");
   }

   return 0;
};
