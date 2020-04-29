
PORT := 8192

.PHONY: run_server run_server_epoll run_client clean
default: run_server_epoll


run_server: server
	./server $(PORT)

run_server_epoll: server_epoll
	./server_epoll $(PORT)

run_server_epoll_bench: server_epoll_bench
	numactl --cpubind=0 --membind=0 ./server_epoll_bench $(PORT) 2

run_server_epoll_reuseport: server_epoll_reuseport
	./server_epoll_reuseport $(PORT) 2

run_server_epoll_handle: server_epoll_handle
	./$< $(PORT)

run_server_epoll_et_ref: server_epoll_et_ref
	./$< 

run_server_epoll_et: server_epoll_et
	./$< $(PORT)

run_client: client
	./client 127.0.0.1 $(PORT)

run_client_epoll_bench: client_epoll_bench
	numactl --cpubind=1 --membind=1 ./client_epoll_bench 127.0.0.1 $(PORT) 1 4 64 1000

run_client_test_epollout: client_test_epollout
	./$< 127.0.0.1 $(PORT)


server: server.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

server_epoll: server_epoll.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

server_epoll_bench: server_epoll_bench.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

server_epoll_reuseport: server_epoll_reuseport.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

server_epoll_handle: server_epoll_handle.cpp
	g++ -o $@ $^ -pthread

server_epoll_et_ref: server_epoll_et_ref.cpp
	g++ -o $@ $^ -pthread

server_epoll_et: server_epoll_et.cpp
	g++ -o $@ $^ -pthread


client: client.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

client_epoll_bench: client_epoll_bench.cpp util.h util.cpp
	g++ -o $@ $^ -pthread

client_test_epollout: client_test_epollout.cpp util.h util.cpp
	g++ -o $@ $^

clean:
	-rm client client_epoll_bench server server_epoll server_epoll_bench
