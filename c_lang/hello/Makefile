


run: build
	./main

run_strace: build
	strace ./main

build: main
main: main.c
	gcc -o $@ $^


