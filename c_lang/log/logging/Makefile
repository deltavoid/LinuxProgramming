

run: main
	./$<

main: main.c logging.h
#	gcc -g -fno-omit-frame-pointer -rdynamic -o $@ $^ 
	gcc -o main.o -g -fno-omit-frame-pointer -rdynamic -c $<
	gcc -o main -rdynamic main.o 

