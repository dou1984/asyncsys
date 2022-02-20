#!/bin/bash


g++ -std=c++17 -o test *.cpp ./proto/*.cc -lasyncsys -lpthread -lhttp_parser -lhiredis_static -lmysqlclient  -lprotobuf -L/usr/local/lib -L/usr/local/lib64 -L../build -I../src 
