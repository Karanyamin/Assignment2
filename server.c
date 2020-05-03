
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
#include <math.h>

long get_file_length(char* file);
char* int_to_string(long num);

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

int write_bytes_to_socket(char* file, int socket){
    FILE* fp = fopen(file, "r");
    if (fp == NULL){
        printf("error in writing bytes to socket\n");
        return 0;
    }
    long size = get_file_length(file);
    if (size == -1){
        return 0;
    } else if (size == 0){
        fclose(fp);
        return 1;
    }
    char* buffer = (char*)malloc(sizeof(char) * size);
    fread(buffer, 1, size, fp);
    write(socket, buffer, size);
    free(buffer);
    fclose(fp);
    return 1;
}

int write_bytes_of_all_files(char* path, int socket){

    DIR* dir = opendir(path);
    struct dirent * handle;

    if (dir == NULL){
        printf("Error opening directory with path name: [%s] in get_number_of_files_in_project\n", path);
        return 0;
    }

    while ((handle = readdir(dir)) != NULL){
        if (!in_ignore_list(handle->d_name)){
            if (handle->d_type == DT_DIR){
                append_file_path(path, handle->d_name);
                if (write_bytes_of_all_files(path, socket) == 0){
                    return 0;
                }
                delete_last_file_path(path, handle->d_name);
            } else if (handle->d_type == DT_REG){
                append_file_path(path, handle->d_name);
                printf("Scanning this file %s for writing bytes to socket\n", path);
                if (write_bytes_to_socket(path, socket) == 0){
                    return 0;
                }
                delete_last_file_path(path, handle->d_name);
            }
        }
    }
    closedir(dir);
    return 1;
}

int get_number_of_files_in_project(char* path, int* total){

    DIR* dir = opendir(path);
    struct dirent * handle;

    if (dir == NULL){
        printf("Error opening directory with path name: [%s] in get_number_of_files_in_project\n", path);
        return 0;
    }

    while ((handle = readdir(dir)) != NULL){
        if (!in_ignore_list(handle->d_name)){
            if (handle->d_type == DT_DIR){
                append_file_path(path, handle->d_name);
                if(get_number_of_files_in_project(path, total) == 0){
                    return 0;
                }
                delete_last_file_path(path, handle->d_name);
            } else if (handle->d_type == DT_REG){
                append_file_path(path, handle->d_name);
                printf("Scanning this file %s for counting number of files\n", path);
                delete_last_file_path(path, handle->d_name);
                (*total)++;
            }
        }
    }
    closedir(dir);
    return 1;
}

int write_all_files_to_socket_in_sequence(char* path, int socket){

    DIR* dir = opendir(path);
    struct dirent * handle;

    if (dir == NULL){
        printf("Error opening directory with path name: [%s] in write_all_files_to_socket_in_sequence\n", path);
        return 0;
    }

    while ((handle = readdir(dir)) != NULL){
        if (!in_ignore_list(handle->d_name)){
            if (handle->d_type == DT_DIR){
                append_file_path(path, handle->d_name);
                if (write_all_files_to_socket_in_sequence(path, socket) == 0){
                    return 0;
                }
                delete_last_file_path(path, handle->d_name);
            } else if (handle->d_type == DT_REG){
                append_file_path(path, handle->d_name);
                printf("Scanning this file %s for appending file to socket\n", path);
                char* length_of_file = int_to_string(strlen(path));
                if (length_of_file == NULL) return 0;
                write(socket, length_of_file, strlen(length_of_file));
                write(socket, path, strlen(path));
                char* bytes_in_file = int_to_string(get_file_length(path));
                if (bytes_in_file == NULL) return 0;
                write(socket, bytes_in_file, strlen(bytes_in_file));
                free(length_of_file);
                free(bytes_in_file);
                delete_last_file_path(path, handle->d_name);
            }
        }
    }
    closedir(dir);
    return 1;
}

void checkout_write_filepath_to_socket(char* path, int socket){
    write(socket, path, strlen(path));
    write(socket,"|", 1);
}

char* int_to_string(long num){ //user must free return char*
    if (num < 0){
        printf("Why did you give int_to_string a non positive number?\n");
        return NULL;
    } else if (num == 0){
        char* buffer = malloc(sizeof(char) * 3);
        buffer[0] = '0';
        buffer[1] = ':';
        buffer[2] = '\0';
        return buffer;
    } else {
        long enough = (long)((ceil(log10(num))+2)*sizeof(char)); //+2 to accomadate for the delimiter and null character
        char* buffer = malloc(sizeof(char) * enough);
        sprintf(buffer, "%ld", num);
        strcat(buffer, ":");
        return buffer;
    }
}

void checkout(char* project, int socket){
    //Error checking
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    if (!project_exists_on_server(project)){
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }
    //writing send command to socket
    write(socket, "sendfile:", 9);
    //Finding out how many files to send
    int number_of_files = 0;
    bzero(path, sizeof(path));
    strcpy(path, "./");
    strcat(path, project);
    if(get_number_of_files_in_project(path, &number_of_files) == 0){
        printf("Error in getting number of files in project\n");
        return;
    }
    char* number_of_files_s = int_to_string(number_of_files);
    if (number_of_files_s == NULL) return;
    write(socket, number_of_files_s, strlen(number_of_files_s));
    free(number_of_files_s);
    //write file path names in sequence
    bzero(path, sizeof(path));
    strcpy(path, "./");
    strcat(path, project);
    if(write_all_files_to_socket_in_sequence(path, socket) == 0){
        printf("Error in writing all filepath names to socket\n");
        return;
    }
    //write all bytes of all files in order
    bzero(path, sizeof(path));
    strcpy(path, "./");
    strcat(path, project);
    write_bytes_of_all_files(path, socket);
    write(socket, ":done:", 7);
    
}

long get_file_length(char* file){
    FILE* fd = fopen(file, "r");
    if (fd == NULL){
        printf("Error in get_file_length trying to open file\n");
        return -1;
    }
    if (fseek(fd, 0L, SEEK_END) != 0){
        printf("Error in get_file_length trying to seek file\n");
        return -1;;
    }

    long result = ftell(fd);
    fclose(fd);
    return result;
}

void create(char* project_name, int socket){
    //Error checking
    char path[NAME_MAX];
    bzero(path, sizeof(path));
    if (project_exists_on_server(project_name)){
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }
    if (mkdir(project_name, 0777) == -1){
        printf("The server failed to make the directory\n");
        return;
    }
    bzero(path, sizeof(path));
    strcpy(path, ".");
    append_file_path(path, project_name);
    append_file_path(path, ".Manifest");
    int fd = open(path, O_WRONLY | O_CREAT, 00600);
    if (fd == -1){
        printf("The server failed to create the Manifest in the directory\n");
        return;
    }
    write(fd, "1\n", 2);
    close(fd);
}

void handle_connection(int socket){
    char buffer[NAME_MAX];
    char project_name[NAME_MAX];
    char arg[NAME_MAX];
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
            read(socket, project_name, sizeof(project_name));
            create(project_name, socket);
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
    if (argc < 2){
        printf("Failed to provide port number\n");
        exit(1);
    }

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
    if (port == 0){
        printf("atoi failed\n");
        exit(1);
    }
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

