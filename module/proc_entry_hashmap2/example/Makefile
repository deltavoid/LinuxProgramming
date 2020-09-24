

default: run_main1


build : main main1

main: main.c
	gcc -o $@ $^

main1: main1.cpp
	g++ -o $@ $^  -pthread

clean:
	-rm main main1


run_main: build
	./main

run_main1: build
	./main1 8 120

