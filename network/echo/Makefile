

.PHONY: run run_client clean

run: server
	./server

run_client: client
	./client 127.0.0.1


server: server.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

client: client.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

clean:
	-rm client server