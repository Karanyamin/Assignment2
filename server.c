
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
#include <dirent.h>
#include <limits.h>

void chatFunction(int socket){
    char* str = malloc(sizeof(char) * 256);
    int n;
    while(true){
        bzero(str, sizeof(char) * 256);
        read(socket, str, sizeof(char) * 256);
        printf("Message sent by Saavi: %sEnter message for Saavi: ", str);
        bzero(str, sizeof(char) * 256);
        n = 0;
        while ((str[n++] = getchar()) != '\n'){
            //Keeps looping until ENTER is pressed
        }
        write(socket, str, sizeof(char) * 256);
        
        if (strncmp(str, "exit", 4) == 0){
            printf("Ending chat session\n");
            break;
        }
    }
    free(str);    
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

bool project_exists_on_server(char* name){
    if (strcmp(name, "ignore") == 0) return true;

    DIR* directory = opendir(".");
    if (directory == NULL){
        printf("error on checking if project %s exists on server\n", name);
        exit(1);
    }
    struct dirent* handle;

    while ((handle = readdir(directory)) != NULL){
        if (strcmp(handle->d_name, name) == 0) return true;
    }
    
    closedir(directory);
    return false;
}

void append_file_path(char* path, char* nextDirectoryName){
    strcat(path, "/");
    strcat(path, nextDirectoryName);
}

void delete_last_file_path(char* path, char* nextDirectoryName){
    int length1 = strlen(path);
    int length2 = strlen(nextDirectoryName);
    bzero(path+(length1 - length2 - 1), length2 + 1);
}

bool in_ignore_list(char* name){
    if (strcmp(name, ".git") == 0){
        return true;
    } else if (strcmp(name, ".") == 0){
        return true;
    } else if (strcmp(name, "..") == 0){
        return true; 
    } else {
        return false;
    }

    /*
    if (strcmp(name, ".vscode") == 0){
        return true;
    } else if (strcmp(name, "temp") == 0){
        return true;
    } else if (strcmp(name, "client") == 0){
        return true;
    } else if (strcmp(name, "getHost") == 0){
        return true;
    }  else if (strcmp(name, "getHost.c") == 0){
        return true;
    } else if (strcmp(name, "Makefile") == 0){
        return true;
    }  else if (strcmp(name, "server") == 0){
        return true;
    } else if (strcmp(name, "server.c") == 0){
        return true;
    } else if (strcmp(name, ".git") == 0){
        return true;
    } else if (strcmp(name, ".") == 0){
        return true;
    } else if (strcmp(name, "..") == 0){
        return true; 
    } else {
        return false;
    }
    */
}

void recursiveBehavior(char* path, void (*fnptr)(char*, int), bool openNewDirectories, int socket){

    DIR* dir = opendir(path);
    struct dirent * handle;

    if (dir == NULL){
        printf("Error opening directory with path name: [%s] in recursive Behavior\n", path);
        exit(1);
    }

    while ((handle = readdir(dir)) != NULL){
        if (!in_ignore_list(handle->d_name)){
            if (handle->d_type == DT_DIR && openNewDirectories){
                append_file_path(path, handle->d_name);
                fnptr(path, socket);
                recursiveBehavior(path, fnptr, openNewDirectories, socket);
                delete_last_file_path(path, handle->d_name);
            } else if (handle->d_type == DT_REG){
                append_file_path(path, handle->d_name);
                printf("Scanning diles [%s]\n", path);
                fnptr(path, socket);
                delete_last_file_path(path, handle->d_name);
            }
        }
    }
    closedir(dir);
}

void checkout_write_filepath_to_socket(char* path, int socket){
    write(socket, path, strlen(path));
    write(socket,"|", 1);
}

void checkout(char* project, int socket){
    //Error checking
    char buffer[256];
    bzero(buffer, sizeof(buffer));
    if (!project_exists_on_server(project)){
        strcpy(buffer, "fail");
        write(socket, buffer, sizeof(buffer));
    } else {
        strcpy(buffer, "success");
        write(socket, buffer, sizeof(buffer));
    }
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    strcpy(path, ".");
    recursiveBehavior(path, checkout_write_filepath_to_socket, true, socket);
    strcpy(buffer, "done|");
    write(socket, buffer, sizeof(buffer));
}

void handle_connection(int socket){
    char buffer[256];
    char project_name[256];
    char arg[256];
    bzero(buffer, sizeof(buffer));
    bzero(project_name, sizeof(project_name));
    bzero(arg, sizeof(arg));

    while(true){
        read(socket, buffer, sizeof(buffer));
        if (strcmp(buffer, "checkout") == 0) {
            read(socket, project_name, sizeof(project_name));
            checkout(project_name, socket);
        } else if (strcmp(buffer, "update") == 0){
            return;
        } else if (strcmp(buffer, "upgrade") == 0){
            return;
        } else if (strcmp(buffer, "commit") == 0){
            return;
        } else if (strcmp(buffer, "push") == 0){
            return;
        } else if (strcmp(buffer, "create") == 0){
            return;
        } else if (strcmp(buffer, "destory") == 0){
            return;
        } else if (strcmp(buffer, "currentversion") == 0){
            return;
        } else if (strcmp(buffer, "history") == 0){
            return;
        } else if (strcmp(buffer, "rollback") == 0){
            return;
        } else if (strcmp(buffer, "done") == 0){
            break;
        }
        bzero(buffer, sizeof(buffer));
    }

    close(socket);
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
    int port = atoi(argv[1]);
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        printf("Binding Port: FAILED\n");
        exit(1);
    } else {
        printf("Binding Port: SUCCESS\n");
    }
    if (listen(socketFD, 100) < 0){
        printf("Making server socket Listening type: FAILED\n");
        exit(1);
    } else {
        printf("Making server socket Listening type: SUCCESS\n");
    }
    
    while(true){
        length = sizeof(clientAddress);
        printf("Listening for requests...\n");
        clientSocket = accept(socketFD, (struct sockaddr *)&clientAddress, &length);
        if (clientSocket < 0){
            printf("Connection to client failed");
            exit(1);
        } else {
            printf("Connection to client succeeded\n");
        }
        handle_connection(clientSocket);
    }



    
    chatFunction(clientSocket);
    close(socketFD);
    return 0;
}

