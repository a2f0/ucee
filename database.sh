#!/bin/bash
sqlite3 OrderBook.db "create table t1 (t1key TEXT PRIMARY KEY, account TEXT, user TEXT, ordertype INTEGER, timestamp UNSIGNED BIG INT, side INTEGER, symbol TEXT, price DOUBLE, quantity UNSIGNED BIG INT);"
sqlite3 OrderBook.db "create table t2 (t1key TEXT PRIMARY KEY, account TEXT, user TEXT, ordertype INTEGER, timestamp UNSIGNED BIG INT, side INTEGER, symbol TEXT, price DOUBLE, quantity UNSIGNED BIG INT);"
sqlite3 Reporting.db "create table t1 (symbol TEXT, price DOUBLE, quantity UNSIGNED BIG INT, orderid1 TEXT, orderid2 TEXT);"
