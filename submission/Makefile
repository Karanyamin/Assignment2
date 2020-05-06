all: WTFserver WTF

WTFserver: WTFserver.c
	gcc -g WTFserver.c -o WTFserver -L/usr/local/lib/ -lssl -lcrypto -lm -lpthread
WTF: WTF.c
	gcc -g WTF.c -o WTF -L/usr/local/lib/ -lssl -lcrypto -lm

clean:
	rm -f WTFserver WTF