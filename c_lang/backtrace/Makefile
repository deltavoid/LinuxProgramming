

run: main
	./main

main: main.c backtrace_debug.h
	gcc -g -fno-omit-frame-pointer -rdynamic -o $@ $^ 

clean:
	rm main