
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <openssl/sha.h>

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
		if (strncmp(str, "exit", 4) == 0){
            printf("Ending chat session\n");
            break;
        }
        bzero(str, sizeof(str));
        read(socket, str, sizeof(str));
        printf("Message sent by Karan: %s", str);
    }
	
} 

void calculate_and_update_hash(char* filepath, unsigned char* hash){ //Hash must be of Length SHA_DIGEST_LENGTH
    //unsigned char* hash = malloc(sizeof(char) * SHA_DIGEST_LENGTH);
    char buffer[5000000];
    int fd = open(filepath, O_RDONLY);
    int readed = 0;
    int written = 0;
    bzero(buffer, sizeof(buffer));
    do {
        readed = read(fd, buffer+written, sizeof(buffer)-written);
        written += readed;
    } while (readed > 0);
    
    SHA1(buffer, sizeof(buffer), hash);
    if(hash == NULL){
        printf("HASH NULL\n");
        return;
    }
    //prints out the hash in hexadecimal
    printf("The hash for the file [%s] is [", filepath);
    int i;
    for (i = 0; i < SHA_DIGEST_LENGTH; i++){
        printf("%02x", hash[i]);
    }
    printf("]\n");
}

void valid_command(int argc, char** argv){
    if (argc < 3) exit(1);

    if (strcmp(argv[1], "checkout") == 0 && argc == 3) {
        return;
    } else if (strcmp(argv[1], "update") == 0 && argc == 3){
        return;
    } else if (strcmp(argv[1], "upgrade") == 0 && argc == 3){
        return;
    } else if (strcmp(argv[1], "commit") == 0 && argc == 3){
        return;
    } else if (strcmp(argv[1], "push") == 0 && argc == 3){
        return;
    } else if (strcmp(argv[1], "create") == 0 && argc == 3){
        return;
    } else if (strcmp(argv[1], "destory") == 0 && argc == 3){
        return;
    } else if (strcmp(argv[1], "add") == 0 && argc == 4){
        return;
    } else if (strcmp(argv[1], "remove") == 0 && argc == 4){
        return;
    } else if (strcmp(argv[1], "currentversion") == 0 && argc == 3){
        return;
    } else if (strcmp(argv[1], "history") == 0 && argc == 3){
        return;
    } else if (strcmp(argv[1], "rollback") == 0 && argc == 4){
        return;
    } else if (strcmp(argv[1], "configure") == 0 && argc == 4){
        return;
    } else {
        printf("Invalid command\n");
        exit(1);
    }
}

void testcheckout(int socket){
    char buffer[256];
    char c;
    int n = 0;
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "checkout");
    write(socket, buffer, sizeof(buffer));
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "ignore");
    write(socket, buffer, sizeof(buffer));
    bzero(buffer, sizeof(buffer));
    read(socket, buffer, sizeof(buffer));
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project on server\n");
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("success in finding project on server\n");
    }
    bzero(buffer, sizeof(buffer));
    while (true){
        while(read(socket, &c, 1) != 0 && c != '|'){
            buffer[n++] = c;
        } 
        if (strcmp(buffer, "done") == 0) break;
        printf("filepath %s\n", buffer);
        bzero(buffer, sizeof(buffer));
        n = 0;
    }
    write(socket, "done", 5);
    close(socket);
}

int main(int argc, char** argv){
    valid_command(argc, argv);

    if (strcmp(argv[1], "configure") == 0){
        int fd = open(".configure", O_WRONLY | O_CREAT, 00600);
        if (ftruncate(fd, 0) == -1 || fsync(fd) == -1){
            printf("Error on truncating .configure file to an empty file or syncing file to disk\n");
            exit(1);
        }
        int current_written = 0;
        int total_written = 0;
        do {
            current_written = write(fd, argv[2]+total_written, strlen(argv[2])-total_written);
            total_written += current_written;
        } while (current_written > 0);
        write(fd, " ", 1);
        current_written = 0;
        total_written = 0;
        do {
            current_written = write(fd, argv[3]+total_written, strlen(argv[3])-total_written);
            total_written += current_written;
        } while (current_written > 0);
        close(fd);
    } else {
        if(access(".configure", R_OK) == -1){
            printf("Configure file does not exit, please run configure\n");
            exit(1);
        }
        FILE* fd = fopen(".configure", "r");
        if (fd == NULL){
            printf("error on opening fd\n");
        }
        char hostname[256];
        char portname[256];
        int port;
        bzero(hostname, sizeof(hostname));
        bzero(portname, sizeof(portname));
        int n = 0;
        char c;
        while ((c = getc(fd)) != ' '){
            hostname[n++] = c;
        }
        printf("The hostname is %s\n", hostname);
        n = 0;
        while ((c = getc(fd)) != EOF){
            portname[n++] = c;
        }
        port = atoi(portname);
        printf("The port is %d\n", port);
        fclose(fd);
        int socketFD = socket(AF_INET, SOCK_STREAM, 0);
        if (socketFD == -1){
            printf("Client socket creation: FAILED\n");
            exit(1);
        } else {
            printf("Client socket creation: SUCCESS\n");
        }
        struct hostent* result = gethostbyname(hostname);
        struct sockaddr_in serverAddress;

        bzero((char*)&serverAddress, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        bcopy((char*)result->h_addr, (char*)&serverAddress.sin_addr.s_addr, result->h_length);
        if (connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
            printf("Connecting to Server: FAILED\n");
            exit(1);
        } else {
            printf("Connecting to Server: SUCCESS\n");
        }
        //chatFunction(socketFD);
        testcheckout(socketFD);
    }
    return 0;
}
