

.PHONY: run_server run_server_epoll run_client clean
default: run_server


run_server: server
	./server

#run_server_epoll: server_epoll
#	./server_epoll

run_client: client
	./client fe81::f843:caff:fe2c:60e0 3

server: server.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

#server_epoll: server_epoll.cpp util.h util.cpp
#	g++ -o $@ $^ -pthread

client: client.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

clean:
	-rm client server