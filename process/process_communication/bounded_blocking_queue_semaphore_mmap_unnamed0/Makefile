

.PHONY: run 




run: main
	./main

run_strace: main
	strace ./main
	
main: main.cpp
	g++ -o $@ $< -pthread

clean:
	-rm main share.txt
