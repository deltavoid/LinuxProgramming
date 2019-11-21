

.PHONY: run run_system clean

run: master worker
	./master

run_system: system worker
	./system

master: master.cpp
	g++ -o $@ $<

system: system.cpp
	g++ -o $@ $<

worker: worker.cpp
	g++ -o $@ $<

clean:
	-rm master system worker

