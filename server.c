
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
        read(socket, str, sizeof(str));
        printf("Message sent by Saavi: %sEnter message for Saavi: ", str);
        bzero(str, sizeof(str));
        n = 0;
        while ((str[n++] = getchar()) != '\n'){
            //Keeps looping until ENTER is pressed
        }
        write(socket, str, sizeof(str));
        
        if (strncmp(str, "exit", 4) == 0){
            printf("Ending chat session\n");
            break;
        }
    }
}

int main(int argc, char** argv){
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    int clientSocket;
    int length;
    if (socketFD == -1){
        printf("Server socket creation: FAILED\n");
        exit(1);
    } else {
        printf("Server socket creation: SUCCESS\n");
    }

    struct sockaddr_in serverAddress, clientAddress;

    bzero((char*)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12370);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        printf("Binding Port: FAILED\n");
        exit(1);
    } else {
        printf("Binding Port: SUCCESS\n");
    }
    if (listen(socketFD, 1) < 0){
        printf("Making server socket Listening type: FAILED\n");
        exit(1);
    } else {
        printf("Making server socket Listening type: SUCCESS\n");
    }
    length = sizeof(clientAddress);

    printf("Listening for requests...\n");
    clientSocket = accept(socketFD, (struct sockaddr *)&clientAddress, &length);
    if (clientSocket < 0){
        printf("Connection to client failed");
        exit(1);
    } else {
        printf("Connection to client succeeded\n");
    }
    chatFunction(clientSocket);
    close(socketFD);
    return 0;
}

/*
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 
  
// Function designed for chat between client and server. 
void func(int sockfd) 
{ 
    char buff[MAX]; 
    int n; 
    // infinite loop for chat 
    for (;;) { 
        bzero(buff, MAX); 
  
        // read the message from client and copy it in buffer 
        read(sockfd, buff, sizeof(buff)); 
        // print buffer which contains the client contents 
        printf("From client: %s\t To client : ", buff); 
        bzero(buff, MAX); 
        n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
  
        // and send that buffer to client 
        write(sockfd, buff, sizeof(buff)); 
  
        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
} 
  
// Driver function 
int main() 
{ 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n"); 
  
    // Function for chatting between client and server 
    func(connfd); 
  
    // After chatting close the socket 
    close(sockfd); 
} 
*/