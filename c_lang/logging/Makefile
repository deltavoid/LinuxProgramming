

run: main
	./$<

main: main.c logging.h
	gcc -g -fno-omit-frame-pointer -rdynamic -o $@ $^ 
