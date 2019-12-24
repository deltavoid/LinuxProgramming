
PORT := 8192

.PHONY: run clean

run_server: server
	./server $(PORT)

run_client: client
	./client 127.0.0.1 $(PORT)

run_client_connect: client_connect
	./client_connect 127.0.0.1 $(PORT)

server: server.cpp
	g++ -o $@ $^ 

client: client.cpp
	g++ -o $@ $^

client_connect: client_connect.cpp
	g++ -o $@ $^


clean:
	-rm server client client_connect
