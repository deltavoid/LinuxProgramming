
PORT := 8192

.PHONY: run_server run_server_epoll run_client clean
default: run_server_epoll


run_server: server
	./server $(PORT)

run_server_epoll: server_epoll
	./server_epoll $(PORT)

run_client: client
	./client 127.0.0.1 $(PORT)

run_client_epoll: client_epoll
	./client_epoll 127.0.0.1 $(PORT) 4 1 64 10


server: server.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

server_epoll: server_epoll.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

client: client.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

client_epoll: client_epoll.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

clean:
	-rm client server server_epoll