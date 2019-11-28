

.PHONY: run clean

run_server: server
	./server

run_client: client
	./client 127.0.0.1

run_client_connect: client_connect
	./client_connect 127.0.0.1

server: server.cpp
	g++ -o $@ $^ 

client: client.cpp
	g++ -o $@ $^

client_connect: client_connect.cpp
	g++ -o $@ $^


clean:
	-rm server client client_connect
