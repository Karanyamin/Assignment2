all: server

server: server.c
	gcc -g server.c -o server -L/usr/local/lib/ -lssl -lcrypto
c: ./temp/client.c
	gcc -g ./temp/client.c -o ./temp/client -L/usr/local/lib/ -lssl -lcrypto
clean:
	rm -f wtf