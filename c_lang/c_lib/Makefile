

run_atomic: atomic
	./atomic

run_getenv: getenv
	HELLO=WORLd ./getenv

run_getarg: getarg
	./getarg 1 2 3 4 5 


build: atomic getenv

atomic: atomic.c
	gcc -o $@ $^ -pthread

getenv: getenv.c
	gcc -o $@ $^

getarg: getarg.c
	gcc -o $@ $^


clean:
	-rm atomic getenv getarg
