#!/bin/bash
g++ -o main.o helpermain.cpp
g++ -o matcheng.o matcheng.cpp -Wall -W -O2 -lsqlite3
./matcheng.o
