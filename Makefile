# Makefile for myfind project

all: myfind

myfind: main.cpp
	g++ -g -Wall -std=c++17 -o myfind main.cpp

clean:
	rm -f myfind
