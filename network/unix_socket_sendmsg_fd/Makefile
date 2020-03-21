

.PHONY: run_server run_client

run_server: server
	./server

run_client: client
	./client


server: server.c
	gcc -o $@ $^

client: client.c
	gcc -o $@ $^

clean:
	-rm server server.socket client client.socket
