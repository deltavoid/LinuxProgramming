

.PHONY: run 


run: main
	./main

run_strace: main
	strace ./main

main: main.cpp bounded_blocking_queue.cpp
	g++ -o $@ $^ -pthread

clean:
	main