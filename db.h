#include <stdio.h>
#include <iostream>
#include <string>
#include "sqlite3.h"
#include "messages.h"
#include <list>
#include <cstring>

using namespace std;

int add_row(Order myorder);
int del_row(Order myorder);
int modify_row(Modify mymodify);
list<Order> get_db(std::string dbname, std::string tablename);
