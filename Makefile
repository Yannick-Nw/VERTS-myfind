# Makefile for myfind project

all: myfind

myfind: main.cpp
	g++ -g -Wall -o myfind main.cpp

clean:
	rm -f myfind
