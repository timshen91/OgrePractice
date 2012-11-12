all:
	g++ -std=c++11 -O2 -lOgreMain -lOIS -lboost_system main.cpp

.PHONY : all
