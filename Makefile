all: server client

server: WTFserver.c
	gcc -g WTFserver.c -o WTFserver -L/usr/local/lib/ -lssl -lcrypto -lm -lpthread
client: ./temp/WTF.c
	gcc -g ./temp/WTF.c -o ./temp/WTF -L/usr/local/lib/ -lssl -lcrypto -lm
test: WTFtest.c
	gcc -g WTFtest.c -o WTFtest
clean:
	rm -f WTFserver ./temp/WTF WTFtest
delete: 
	rm -rf client1 client2 server