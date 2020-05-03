
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
#include <limits.h>
#include <errno.h>

typedef struct node {
    char* filePath;
    long length;
    struct node * next;
} node;

void append_file_path(char* path, char* nextDirectoryName){
    strcat(path, "/");
    strcat(path, nextDirectoryName);
}

void delete_last_file_path(char* path, char* nextDirectoryName){
    int length1 = strlen(path);
    int length2 = strlen(nextDirectoryName);
    bzero(path+(length1 - length2 - 1), length2 + 1);
}

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
    if (argc < 2) exit(1);

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
    } else if (strcmp(argv[1], "done") == 0 && argc == 2){
        return;
    } else {
        printf("Invalid command\n");
        exit(1);
    }
}

void printList(node* head){
    while(head != NULL){
        printf("Filepath [%s] bytes [%ld]\n", head->filePath, head->length);
        head = head->next;
    }
}

node* append_To_List(node* head, char* filepath, long length){
    node* temp = malloc(sizeof(node));
    temp->filePath = malloc(sizeof(char) * PATH_MAX);
    strcpy(temp->filePath, filepath);
    temp->length = length;
    temp->next = NULL;

    if (head == NULL) return temp;

    node * ptr = head;

    while (ptr->next != NULL){
        ptr = ptr->next;
    }
    ptr->next = temp;
    return head;
}

bool isFile(char* file){
    int i = 0;
    while (file[i] != '\0'){
        if (file[i++] == '.') return true;
    }
    return false;
}

int makedir(const char *path)
{
    const size_t len = strlen(path);
    char _path[PATH_MAX];
    char *p; 
    errno = 0;
    if (len > sizeof(_path)-1) {
        errno = ENAMETOOLONG;
        return -1; 
    }   
    strcpy(_path, path);
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            if (mkdir(_path, S_IRWXU) != 0) {
                if (errno != EEXIST)
                    return -1; 
            }

            *p = '/';
        }
    }   
    if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
            return -1; 
    }   
    return 0;
}

void create_Directory_Path(char* path){
    char copy[PATH_MAX];
    bzero(copy, sizeof(copy));
    strcpy(copy, path);
    const char delim[2] = "/";
    char* token = strtok(copy, delim);
    int directory_elements = 0;
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "./");

    while ((token = strtok(NULL, delim)) != NULL){
        if (!isFile(token)){
            append_file_path(buffer, token);
        } else {
            makedir(buffer);
            return;
        }
    }
}

void testcheckout(int socket, char* project_name){
    char buffer[PATH_MAX];
    char c;
    int n = 0;
    bzero(buffer, sizeof(buffer));
    read(socket, buffer, sizeof(buffer));
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project on server\n");
        close(socket);
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("success in finding project on server\n");
    }
    bzero(buffer, sizeof(buffer));
    bool sendfile = false;
    while (true){
        while(read(socket, &c, 1) != 0 && c != ':'){
            buffer[n++] = c;
        } 
        if (strcmp(buffer, "done") == 0) break;
        else if (strcmp(buffer, "sendfile") == 0) sendfile = true;
        //printf("%s\n", buffer);
        if (sendfile){
            int number_of_files = 0;
            char* eptr;
            bzero(buffer, sizeof(buffer));
            n = 0;
            while(read(socket, &c, 1) != 0 && c != ':'){
                buffer[n++] = c;
            }
            number_of_files = strtol(buffer, &eptr, 0); 
            if (number_of_files == 0){
                if (errno == EINVAL || errno == ERANGE){
                    printf("Conversion error occurred: %d\n", errno);
                    write(socket, "done", 5);
                    close(socket);
                    exit(0);
                } 
            }
            //make a linked list with number_of_files many nodes
            node * head = NULL;
            while (number_of_files > 0){
                int file_name_length = 0;
                char filepath[PATH_MAX];
                long number_of_bytes = 0;
                bzero(buffer, sizeof(buffer));
                bzero(filepath, sizeof(filepath));
                n = 0;
                while(read(socket, &c, 1) != 0 && c != ':'){
                    buffer[n++] = c;
                }
                file_name_length = strtol(buffer, &eptr, 0);
                if (file_name_length == 0){
                    if (errno == EINVAL || errno == ERANGE){
                        printf("Conversion error occurred: %d\n", errno);
                        write(socket, "done", 5);
                        close(socket);
                        exit(0);
                    } 
                }
                bzero(buffer, sizeof(buffer));
                read(socket, buffer, file_name_length);
                strcpy(filepath, buffer);
                bzero(buffer, sizeof(buffer));
                n = 0;
                while(read(socket, &c, 1) != 0 && c != ':'){
                    buffer[n++] = c;
                }
                number_of_bytes = strtol(buffer, &eptr, 0);
                if (number_of_bytes == 0){
                    if (errno == EINVAL || errno == ERANGE){
                        printf("Conversion error occurred: %d\n", errno);
                        write(socket, "done", 5);
                        close(socket);
                        exit(0);
                    } 
                }
                head = append_To_List(head, filepath, number_of_bytes);
                number_of_files--;
            }
            printList(head);
            while (head != NULL){
                create_Directory_Path(head->filePath);
                int fd = open(head->filePath, O_WRONLY | O_CREAT, 00600);
                if (fd == -1){
                    printf("Error on opening filepath %s\n", head->filePath);
                    write(socket, "done", 5);
                    close(socket);
                    exit(1);
                }
                char* bytes = malloc(sizeof(char) * head->length);
                int bytes_read = 0, len = 0;
                while (bytes_read < head->length && ((len = recv(socket, bytes + bytes_read, head->length-bytes_read, 0)) > 0)) {
                    bytes_read += len;
                }
                //long enough = read(socket, bytes, head->length);
                //printf("I read %ld\n", enough);
                int current_write = 0;
                int total_write = 0;
                do {
                    current_write = write(fd, bytes+total_write, head->length - total_write);
                    total_write += current_write; 
                } while (current_write > 0);
                close(fd);
                head = head->next;
            }
            sendfile = false;
        }
        bzero(buffer, sizeof(buffer));
        n = 0;
    }
    write(socket, "done", 5);
}

void handle_connection(int socket, char** argv){
    char command[NAME_MAX];
    char project_name[NAME_MAX];
    char arg[NAME_MAX];
    bzero(command, sizeof(command));
    bzero(project_name, sizeof(project_name));
    bzero(arg, sizeof(arg));

    if (strcmp(argv[1], "checkout") == 0) {
        strcpy(command, argv[1]);
        strcpy(project_name, argv[2]);
        write(socket, command, sizeof(command));
        write(socket, project_name, sizeof(project_name));
        testcheckout(socket, project_name);
    } else if (strcmp(argv[1], "update") == 0){
        return;
    } else if (strcmp(argv[1], "upgrade") == 0){
        return;
    } else if (strcmp(argv[1], "commit") == 0){
        return;
    } else if (strcmp(argv[1], "push") == 0){
        return;
    } else if (strcmp(argv[1], "create") == 0){
        return;
    } else if (strcmp(argv[1], "destory") == 0){
        return;
    } else if (strcmp(argv[1], "currentversion") == 0){
        return;
    } else if (strcmp(argv[1], "history") == 0){
        return;
    } else if (strcmp(argv[1], "rollback") == 0){
        return;
    } else if (strcmp(argv[1], "done") == 0){
        strcpy(command, argv[1]);
        write(socket, command, sizeof(command));
    }
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
        handle_connection(socketFD, argv);
    }
    
    return 0;
}
