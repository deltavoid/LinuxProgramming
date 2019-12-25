

.PHONY: run_server run_server_epoll run_client clean
default: run_server_epoll


run_server: server
	./server

run_server_epoll: server_epoll
	./server_epoll

run_client: client
	./client 192.168.1.2


server: server.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

server_epoll: server_epoll.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

client: client.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

clean:
	-rm client server server_epoll