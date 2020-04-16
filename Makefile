all: server

server: server.c
	gcc -g server.c -o server
c: client.c
	gcc -g client.c -o client
clean:
	rm -f wtf