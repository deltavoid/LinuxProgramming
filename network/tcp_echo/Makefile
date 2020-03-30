
PORT := 8192

.PHONY: run_server run_server_epoll run_client clean
default: run_server_epoll


run_server: server
	./server $(PORT)

run_server_epoll: server_epoll
	./server_epoll $(PORT)

run_server_epoll1: server_epoll1
	numactl --cpubind=0 --membind=0 ./server_epoll1 $(PORT) 2

run_client: client
	./client 127.0.0.1 $(PORT)

run_client_epoll: client_epoll
	numactl --cpubind=1 --membind=1 ./client_epoll 127.0.0.1 $(PORT) 1 4 64 1000


server: server.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

server_epoll: server_epoll.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

server_epoll1: server_epoll1.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

client: client.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

client_epoll: client_epoll.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

clean:
	-rm client client_epoll server server_epoll server_epoll1 
