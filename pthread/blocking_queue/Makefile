

.PHONY: run clean

run_strace: main
	strace ./main

run: main
	./main

#main: blocking_queue_condvar.h
main: blocking_queue_semaphore.h
main: main.cpp
	g++ -o $@ $< -pthread



clean:
	rm main