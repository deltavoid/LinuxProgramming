

default: run_main1


build : main main1

main: main.c
	gcc -o $@ $^

main1: main1.cpp
	g++ -o $@ $^  -pthread


run_main: build
	./main

run_main1: build
	./main1 2 60