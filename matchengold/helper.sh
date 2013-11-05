#!/bin/bash
g++ -o main.o helpermain.cpp
g++ -o matcheng.o matcheng.cpp -Wall -O2
./matcheng.o
