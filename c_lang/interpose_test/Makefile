


run: main libinterpose.so
	LD_PRELOAD=./libinterpose.so ./main


main: main.c
	cc -o $@ $^

interpose.shared.o: interpose.c
	cc  -std=gnu99 -O3 -g -Wall -Werror -march=native -fno-omit-frame-pointer  -fPIC  -c -o interpose.shared.o interpose.c

libinterpose.so: interpose.shared.o
	cc -pthread -g  -shared interpose.shared.o -lm -lpthread -lrt -ldl  -o libinterpose.so


