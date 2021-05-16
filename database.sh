#!/bin/bash
#
#
# First  Table (t1): Reflects State of the Order Book at any given time (orders will be deleted when filled, modified if appropriate)
# Second Table (t2): Records All Orders (orders are never removed)
# Third  Table (t3): Records All Trades (trades are never removed)
#
sqlite3 OrderBook.db "drop table if exists t1;"
sqlite3 OrderBook.db "create table t1 (t1key TEXT PRIMARY KEY, account TEXT, user TEXT, ordertype INTEGER, timestamp UNSIGNED BIG INT, side INTEGER, symbol TEXT, price TEXT, quantity UNSIGNED BIG INT);"
sqlite3 OrderBook.db "drop table if exists t2;"
sqlite3 OrderBook.db "create table t2 (t1key TEXT PRIMARY KEY, account TEXT, user TEXT, ordertype INTEGER, timestamp UNSIGNED BIG INT, side INTEGER, symbol TEXT, price TEXT, quantity UNSIGNED BIG INT);"
sqlite3 OrderBook.db "drop table if exists t3;"
sqlite3 OrderBook.db "create table t3 (symbol TEXT, timestamp UNSIGNED BIG INT, price DOUBLE, quantity UNSIGNED BIG INT, orderid1 TEXT, orderid2 TEXT);"
sqlite3 OrderBook.db < triggers.sql
