

.PHONY: run clean

run_server: server
	./server

run_client: client
	./client 127.0.0.1

server: server.cpp
	g++ -o $@ $^ 

client: client.cpp
	g++ -o $@ $^

clean:
	-rm server client
