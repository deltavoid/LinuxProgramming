
PORT := 8192

.PHONY: run clean

run_server: server
	./server $(PORT)

run_client: client
	./client ::1 $(PORT)

server: server.cpp
	g++ -o $@ $^ 

client: client.cpp
	g++ -o $@ $^

clean:
	-rm server client
