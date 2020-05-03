all: server

server: server.c
	gcc -g server.c -o server -L/usr/local/lib/ -lssl -lcrypto -lm
c: ./temp/client.c
	gcc -g ./temp/client.c -o ./temp/client -L/usr/local/lib/ -lssl -lcrypto -lm
clean:
	rm -f wtf