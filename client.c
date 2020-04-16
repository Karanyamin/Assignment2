
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

void chatFunction(int socket){
    char str[256];
    int n;
    while(true){
        bzero(str, sizeof(str));
        n = 0;
        printf("Enter message to Karan: ");
        while ((str[n++] = getchar()) != '\n'){
            //Keeps looping until ENTER is pressed
        }
        write(socket, str, sizeof(str));
        bzero(str, sizeof(str));
        read(socket, str, sizeof(str));
        printf("Message sent by Karan: %s", str);
        if (strncmp(str, "exit", 4) == 0){
            printf("Ending chat session\n");
            break;
        }
    }
}

int main(int argc, char** argv){
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1){
        printf("Client socket creation: FAILED\n");
        exit(1);
    } else {
        printf("Client socket creation: SUCCESS\n");
    }
    struct hostent* result = gethostbyname("127.0.0.1");
    struct sockaddr_in serverAddress;

    bzero((char*)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12370);
    bcopy((char*)result->h_addr, (char*)&serverAddress.sin_addr.s_addr, result->h_length);
    if (connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
        printf("Connecting to Server: FAILED\n");
        exit(1);
    } else {
        printf("Connecting to Server: SUCCESS\n");
    }
    chatFunction(socketFD);
    return 0;
}
/*
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 
void func(int sockfd) 
{ 
	char buff[MAX]; 
	int n; 
	for (;;) { 
		bzero(buff, sizeof(buff)); 
		printf("Enter the string : "); 
		n = 0; 
		while ((buff[n++] = getchar()) != '\n') 
			; 
		write(sockfd, buff, sizeof(buff)); 
		bzero(buff, sizeof(buff)); 
		read(sockfd, buff, sizeof(buff)); 
		printf("From Server : %s", buff); 
		if ((strncmp(buff, "exit", 4)) == 0) { 
			printf("Client Exit...\n"); 
			break; 
		} 
	} 
} 

int main() 
{ 
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("connected to the server..\n"); 

	// function for chat 
	func(sockfd); 

	// close the socket 
	close(sockfd); 
} 
*/
