

run: main
	./main 10 20 30

run_eventfd_exit_test: eventfd_exit_test
	./$<


main: main.c
	gcc -o $@ $^

eventfd_exit_test: eventfd_exit_test.c
	gcc -o $@ $^ -pthread
