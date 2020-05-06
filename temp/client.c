
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <arpa/inet.h>

long get_file_length(char* file);
bool is_file_empty(char* file);
void testadd(char* project, char* filename);
void change_update(FILE* update, char* name);
char* int_to_string(long num, bool delim);
void update_manifest_version(char* project, char* newversion);

typedef struct node {
    char* filePath;
    long length;
    struct node * next;
} node;

typedef struct manifestNode {
    int version;
    char* project;
    char* file;
    char* hash;
    struct manifestNode * next;
} manifestNode;

void append_file_path(char* path, char* nextDirectoryName){
    strcat(path, "/");
    strcat(path, nextDirectoryName);
}

void delete_last_file_path(char* path, char* nextDirectoryName){
    int length1 = strlen(path);
    int length2 = strlen(nextDirectoryName);
    bzero(path+(length1 - length2 - 1), length2 + 1);
}

bool project_exists_on_client(char* name){
    if (strcmp(name, "ignore") == 0) return true;

    DIR* directory = opendir(".");
    if (directory == NULL){
        printf("error on checking if project %s exists on server\n", name);
        exit(1);
    }
    struct dirent* handle;

    while ((handle = readdir(directory)) != NULL){
        if (strcmp(handle->d_name, name) == 0){
            printf("Project exists on client\n");
            return true;
        }
    }
    
    closedir(directory);
    return false;
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
    /*
    //prints out the hash in hexadecimal
    printf("The hash for the file [%s] is [", filepath);
    int i;
    for (i = 0; i < SHA_DIGEST_LENGTH; i++){
        printf("%02x", hash[i]);
    }
    printf("]\n");
    */
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
    } else if (strcmp(argv[1], "destroy") == 0 && argc == 3){
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

bool file_exists(char* filename){
    int fd = open(filename, O_RDWR);
    if (fd == -1){
        printf("File %s does not exist\n", filename);
        return false;
    }
    close(fd);
    return true;
}

void printList(node* head){
    while(head != NULL){
        printf("Filepath [%s] bytes [%ld]\n", head->filePath, head->length);
        head = head->next;
    }
}

void print_manifest_List(manifestNode* head){
    while(head != NULL){
        printf("Version [%d] Project [%s] Filepath [%s] hash [%s]\n", head->version, head->project, head->file, head->hash);
        head = head->next;
    }
}

node* append_To_List(node* head, char* filepath, long length){
    node* temp = malloc(sizeof(node));
    temp->filePath = malloc(sizeof(char) * PATH_MAX);
    bzero(temp->filePath, PATH_MAX);
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

manifestNode* insert_To_Manifest_List(manifestNode* head, int version, char* project, char* filepath, char* hash){
    manifestNode* temp = malloc(sizeof(manifestNode));
    temp->version = version;
    temp->next = head;

    temp->project = malloc(sizeof(char) * NAME_MAX);
    temp->file = malloc(sizeof(char) * PATH_MAX);
    temp->hash = malloc(sizeof(char) * NAME_MAX);

    bzero(temp->project, NAME_MAX);
    bzero(temp->file, PATH_MAX);
    bzero(temp->hash, NAME_MAX);

    strcpy(temp->project, project);
    strcpy(temp->file, filepath);
    strcpy(temp->hash, hash);

    return temp;
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

void sendFile(int socket){
    char buffer[PATH_MAX];
    char c;
    int number_of_files = 0;
    char* eptr;
    bzero(buffer, sizeof(buffer));
    int n = 0;
    while(read(socket, &c, 1) != 0 && c != ':'){
        buffer[n++] = c;
    }
    number_of_files = strtol(buffer, &eptr, 0); 
    if (number_of_files == 0){
        if (errno == EINVAL || errno == ERANGE){
            printf("Conversion error occurred: %d\n", errno);
            write(socket, "done", 5);
            close(socket);
            exit(1);
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
                exit(1);
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
                exit(1);
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
        int current_write = 0;
        int total_write = 0;
        do {
            current_write = write(fd, bytes+total_write, head->length - total_write);
            total_write += current_write; 
        } while (current_write > 0);
        close(fd);
        head = head->next;
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
        write(socket, "done", 5);
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
            sendFile(socket);
            sendfile = false;
        }
        bzero(buffer, sizeof(buffer));
        n = 0;
    }
    write(socket, "done", 5);
}

void testcreate(int socket, char* project_name){
    char buffer[NAME_MAX];
    char c;
    int n = 0;
    bzero(buffer, sizeof(buffer));
    read(socket, buffer, sizeof(buffer));
    if (strcmp(buffer, "fail") == 0){
        printf("project already exists on server\n");
        write(socket, "done", 5);
        close(socket);
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("project does not exist on create...attempting to create project\n");
    }
    //Project has been created on server, time to use checkout to copy it over to our local machine
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "checkout");
    write(socket, buffer, sizeof(buffer));
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, project_name);
    write(socket, buffer, sizeof(buffer));
    testcheckout(socket, project_name);
}

void manifest_case_selection(int version, char* project, char* file, char* hash, manifestNode * ptr, FILE* update, FILE* conflict){
    manifestNode * head = ptr; //Only here for redundancy
    bool found = false;
    while (ptr != NULL){
        if (strcmp(ptr->project, project) == 0 && strcmp(ptr->file, file) == 0){
            if (version != ptr->version || strcmp(ptr->hash, hash) != 0){
                //The live hash needs to match the one stored in client.
                unsigned char temphash[SHA_DIGEST_LENGTH];
                char livehash[PATH_MAX];
                bzero(livehash, sizeof(livehash));

                calculate_and_update_hash(file, temphash);
                int len = 0;
                int i;
                for (i = 0; i < SHA_DIGEST_LENGTH; i++){
                    len += sprintf(livehash+len, "%02x", temphash[i]);
                }
                //compare live hash to hash in client manifest
                if (strcmp(livehash, hash) == 0){
                    bzero(livehash, sizeof(livehash));
                    strcpy(livehash, "M ");
                    strcat(livehash, file);
                    printf("%s\n", livehash);
                    strcat(livehash, " ");
                    strcat(livehash, ptr->hash);
                    strcat(livehash, " ");
                    char* version_s = int_to_string(ptr->version, false);
                    strcat(livehash, version_s);
                    free(version_s);
                    strcat(livehash, "\n");
                    fwrite(livehash, 1, strlen(livehash), update);
                    strcpy(ptr->project, "done");
                    return;
                } else if (strcmp(hash, livehash) != 0 && strcmp(hash, ptr->hash) != 0){
                    char buffer[PATH_MAX];
                    bzero(buffer, sizeof(buffer));
                    strcpy(buffer, livehash);
                    bzero(livehash, sizeof(livehash));
                    strcpy(livehash, "C ");
                    strcat(livehash, file);
                    printf("%s\n", livehash);
                    strcat(livehash, " ");
                    strcat(livehash, buffer);
                    strcat(livehash, "\n");
                    fwrite(livehash, 1, strlen(livehash), conflict);
                    strcpy(ptr->project, "done");
                    return;
                }
            }
            found = true;
            strcpy(ptr->project, "done");
        } else if (strlen(project) == 0){ //We reached the end of the client manifest, any nodes in the server linked list without a done as their project name are files not in client manifest.
            while (head != NULL){
                if (strcmp(head->project, "done") != 0 && strlen(head->file) != 0){
                    char buffer[PATH_MAX];
                    bzero(buffer, sizeof(buffer));
                    strcpy(buffer, "A ");
                    strcat(buffer, head->file);
                    printf("%s\n", buffer);
                    strcat(buffer, " ");
                    strcat(buffer, head->hash);
                    strcat(buffer, " ");
                    char* version_s = int_to_string(head->version, false);
                    strcat(buffer, version_s);
                    free(version_s);
                    strcat(buffer, "\n");
                    fwrite(buffer, 1, strlen(buffer), update);
                }
                head = head->next;
            }
            return;
        }
        ptr = ptr->next;
    }
    //If we searched through the server manifest and did not find the file that means we need to add a D to update
    if (!found){
        char buffer[PATH_MAX];
        bzero(buffer, sizeof(buffer));
        strcpy(buffer, "D ");
        strcat(buffer, file);
        printf("%s\n", buffer);
        strcat(buffer, " ");
        strcat(buffer, hash);
        strcat(buffer, " ");
        char* version_s = int_to_string(version, false);
        strcat(buffer, version_s);
        free(version_s);
        strcat(buffer, "\n");
        fwrite(buffer, 1, strlen(buffer), update);
    }
}

void get_client_and_server_manifests(manifestNode** serverhead, manifestNode** clienthead, int socket, char* project){
    char buffer[PATH_MAX];
    char* eptr;
    bzero(buffer, sizeof(buffer));
    int n = 0;
    char c;
    while(read(socket, &c, 1) != 0 && c != ':'){
        buffer[n++] = c;
    }
    long number_of_bytes_SM = strtol(buffer, &eptr, 0);
    if (number_of_bytes_SM == 0){
        printf("Conversion error occurred: %d\n", errno);
        write(socket, "done", 5);
        close(socket);
        exit(1);
    }
    bzero(buffer, sizeof(buffer));
    n = 0;
    //Will create a Linked list with each line in the server manifest represented as a node
    while(number_of_bytes_SM > 0 && read(socket, &c, 1) != 0){
        //printf("%c", c);
        if (c == '\n'){
            char projectName[NAME_MAX];
            char filepath[PATH_MAX];
            char hash[NAME_MAX];
            bzero(projectName, sizeof(projectName));
            bzero(filepath, sizeof(project));
            bzero(hash, sizeof(project));
            char delim[2] = " ";
            char* token = strtok(buffer, delim);
            int version = atoi(token);
            token = strtok(NULL, delim);
            if (token != NULL){
                strcpy(projectName, token);
            }
            token = strtok(NULL, delim);
            if (token != NULL){
                strcpy(filepath, token);
            }
            token = strtok(NULL, delim);
            if (token != NULL){
                strcpy(hash, token);
            }
            *serverhead = insert_To_Manifest_List(*serverhead, version, projectName, filepath, hash);
            bzero(buffer, sizeof(buffer));
            n = 0;
        } else {
            buffer[n++] = c;
        }
        number_of_bytes_SM--;
    }
    //Will create a linked list same as above but for client manifest now
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    int fd = open(buffer, O_RDONLY);
    if (fd == -1){
        printf("error in opening client manifest\n");
        write(socket, "done", 5);
        exit(1);
    }
    number_of_bytes_SM = get_file_length(buffer);
    bzero(buffer, sizeof(buffer));
    n = 0;
    while(number_of_bytes_SM > 0 && read(fd, &c, 1) != 0){
        //printf("%c", c);
        if (c == '\n'){
            char projectName[NAME_MAX];
            char filepath[PATH_MAX];
            char hash[NAME_MAX];
            bzero(projectName, sizeof(projectName));
            bzero(filepath, sizeof(project));
            bzero(hash, sizeof(project));
            char delim[2] = " ";
            char* token = strtok(buffer, delim);
            int version = atoi(token);
            token = strtok(NULL, delim);
            if (token != NULL){
                strcpy(projectName, token);
            }
            token = strtok(NULL, delim);
            if (token != NULL){
                strcpy(filepath, token);
            }
            token = strtok(NULL, delim);
            if (token != NULL){
                strcpy(hash, token);
            }
            *clienthead = insert_To_Manifest_List(*clienthead, version, projectName, filepath, hash);
            bzero(buffer, sizeof(buffer));
            n = 0;
        } else {
            buffer[n++] = c;
        }
        number_of_bytes_SM--;
    }
    close(fd);
}

void change_update(FILE* update, char* name){
    long length = ftell(update);
    char* temp = malloc(sizeof(char) * (length+1));
    bzero(temp, sizeof(char) * (length+1));
    rewind(update);
    fread(temp, 1, length, update);
    char* delim = "\n";
    char* token = strtok(temp, delim);
    token = strtok(NULL, delim);
    if (token == NULL){
        int fd = open(name, O_RDWR | O_TRUNC);
        close(fd);
    }
}

void testupdate(int socket, char* project){
    char buffer[PATH_MAX];
    char c;
    int n = 0;
    bzero(buffer, sizeof(buffer));
    read(socket, buffer, sizeof(buffer));
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project on server\n");
        write(socket, "done", 5);
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
            manifestNode* serverhead = NULL;
            manifestNode* clienthead = NULL;
            get_client_and_server_manifests(&serverhead, &clienthead, socket, project);
            manifestNode * ptr_s = serverhead;
            manifestNode * ptr_c = clienthead;
            while (ptr_s->next != NULL){
                ptr_s = ptr_s->next;
            }
            while (ptr_c->next != NULL){
                ptr_c = ptr_c->next;
            }
            if (ptr_s->version == ptr_c->version){ //Both manifest are on same version, write blank .Update and delete .Conflict if exists
                bzero(buffer, sizeof(buffer));
                strcpy(buffer, ".");
                append_file_path(buffer, project);
                append_file_path(buffer, ".Update");
                int updateFD = open(buffer, O_RDWR | O_CREAT | O_TRUNC, 00600);
                close(updateFD);
                delete_last_file_path(buffer, ".Update");
                append_file_path(buffer, ".Conflict");
                remove(buffer);
                printf("Up to Date\n");
            } else { //test for the three partial cases
                int version = ptr_s->version;
                char* version_s = int_to_string(version, true);
                ptr_c = clienthead;
                bzero(buffer, sizeof(buffer));
                strcpy(buffer, ".");
                append_file_path(buffer, project);
                append_file_path(buffer, ".Update");
                FILE* update = fopen(buffer, "w+");
                version_s[strlen(version_s) - 1] = '\n';
                fputs(version_s, update);
                delete_last_file_path(buffer, ".Update");
                append_file_path(buffer, ".Conflict");
                FILE* conflict = fopen(buffer, "w+");
                while (ptr_c != NULL){
                    manifest_case_selection(ptr_c->version, ptr_c->project, ptr_c->file, ptr_c->hash, serverhead, update, conflict);
                    ptr_c = ptr_c->next;
                }
                
                if (is_file_empty(buffer)){ //Checks if .Conflict is empty
                    remove(buffer);
                    //Add project version to .Update
                    delete_last_file_path(buffer, ".Conflict");
                    append_file_path(buffer, ".Update");
                    change_update(update, buffer);
                } else {
                    //.Conflict is not empty, remove .Update
                    delete_last_file_path(buffer, ".Conflict");
                    append_file_path(buffer, ".Update");
                    remove(buffer);
                }
                
                fclose(update);
                fclose(conflict);
            }
            sendfile = false;
        }
        bzero(buffer, sizeof(buffer));
        n = 0;
    }
}

bool is_file_empty(char* file){ //If file does not exist it will return true
    FILE* fp = fopen(file, "r+");
    if (fp == NULL){
        //File does not exist
        return true;
    } else {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        if (size == 0){
            return true;
        } else {
            return false;
        }
    }
}

char* int_to_string(long num, bool delim){ //user must free return char*
    if (num < 0){
        printf("Why did you give int_to_string a non positive number?\n");
        return NULL;
    } else if (num == 0){
        if (delim){
            char* buffer = malloc(sizeof(char) * 3);
            buffer[0] = '0';
            buffer[1] = ':';
            buffer[2] = '\0';
            return buffer;
        } else {
            char* buffer = malloc(sizeof(char) * 2);
            buffer[0] = '0';
            buffer[1] = '\0';
            return buffer;
        }
    } else {
        if (delim){
            long enough = (long)((ceil(log10(num))+2)*sizeof(char)); //+2 to accomadate for the delimiter and null character
            char* buffer = malloc(sizeof(char) * enough);
            sprintf(buffer, "%ld", num);
            strcat(buffer, ":");
            return buffer;
        } else {
            long enough = (long)((ceil(log10(num))+1)*sizeof(char)); //+1 to accomadate for the null character
            char* buffer = malloc(sizeof(char) * enough);
            sprintf(buffer, "%ld", num);
            return buffer;
        }
    }
}

int write_bytes_to_socket(char* file, int socket, bool single){
    if (single){
        write(socket, "sendfile:", 9);
        char* length_of_file = int_to_string(get_file_length(file), true);
        write(socket, length_of_file, strlen(length_of_file));
    }
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

void delete_line_from_manifest(char* project, char* file, char* hash){
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    long length = get_file_length(buffer);
    char* oldstream = malloc(sizeof(char) * (length+1));
    bzero(oldstream, sizeof(char) * (length+1));
    FILE* manifestFD = fopen(buffer, "r");
    char buffercopy[PATH_MAX];
    strcpy(buffercopy, buffer);
    if (manifestFD == NULL){
        printf("error in opening manifest\n");
        return;
    }
    char* newstream = malloc(sizeof(char) * (length+1));
    bzero(newstream, sizeof(char) * (length+1));
    bzero(buffer, sizeof(buffer));
    fgets(buffer, length, manifestFD); //Throwing away first link
    strcat(newstream, buffer);
    if (buffer[strlen(buffer) - 1] != '\n'){
        strcat(newstream, "\n");
    }
    bzero(buffer, sizeof(buffer));

    while (fgets(buffer, length, manifestFD) != NULL){
        char copy[PATH_MAX];
        strcpy(copy, buffer);
        bool p = false, f = false, h = false; //Each bool corresponds to the function parameters
        char* delim = " \n";
        char* token = strtok(buffer, delim); //token holds version number

        token = strtok(NULL, delim);
        if (strcmp(token, project) == 0) p = true;
        token = strtok(NULL, delim);
        if (strcmp(token, file) == 0) f = true;
        token = strtok(NULL, delim);
        if (strcmp(token, hash) == 0) h = true;

        if (!p || !f || !h){
            strcat(newstream, copy);
            if (copy[strlen(copy) - 1] != '\n'){
                strcat(newstream, "\n");
            }
        } 
    }   
    
    FILE* newManifestFD = fopen(buffercopy, "w");
    if (newManifestFD == NULL){
        printf("error in opening new manifest\n");
        return;
    }
    int newSize = strlen(newstream);
    fwrite(newstream, 1, newSize, newManifestFD);
    fclose(manifestFD);
    fclose(newManifestFD);
}

void increment_version_number(int version, char* project, char* file, char* hash){
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    long length = get_file_length(buffer);
    char* oldstream = malloc(sizeof(char) * (length+1));
    bzero(oldstream, sizeof(char) * (length+1));
    FILE* manifestFD = fopen(buffer, "r");
    if (manifestFD == NULL){
        printf("error in opening manifest\n");
        return;
    }
    fread(oldstream, 1, length, manifestFD);

    char* token = strtok(oldstream, "\n");
    char* newstream = malloc(sizeof(char) * (length+1));
    bzero(newstream, sizeof(char) * (length+1));
    strcat(newstream, token);
    strcat(newstream, "\n");
    
    while((token = strtok(NULL, "\n")) != NULL){
        if(strstr(token, project) != NULL && strstr(token, file) != NULL){
            char* version_incremented_s = int_to_string(version, false);
            strcat(newstream, version_incremented_s);
            strcat(newstream, " ");
            strcat(newstream, project);
            strcat(newstream, " ");
            strcat(newstream, file);
            strcat(newstream, " ");
            strcat(newstream, hash);
            strcat(newstream, "\n");
            free(version_incremented_s);
        } else {
            strcat(newstream, token);
            strcat(newstream, "\n");
        }
    }
    FILE* newManifestFD = fopen(buffer, "w");
    if (newManifestFD == NULL){
        printf("error in opening new manifest\n");
        return;
    }
    int newSize = strlen(newstream);
    fwrite(newstream, 1, newSize, newManifestFD);
    fclose(manifestFD);
    fclose(newManifestFD);
}

void manifest_case_selection_commit(int version, char* project, char* file, char* hash, manifestNode * ptr, FILE* commit, int socket){
    manifestNode * head = ptr; //Only here for redundancy
    bool found = false;
    while (ptr != NULL){
        if (strcmp(ptr->project, project) == 0 && strcmp(ptr->file, file) == 0){
            if (strcmp(ptr->hash, hash) == 0){
                //The live hash needs to match the one stored in client.
                unsigned char temphash[SHA_DIGEST_LENGTH];
                char livehash[PATH_MAX];
                bzero(livehash, sizeof(livehash));

                calculate_and_update_hash(file, temphash);
                int len = 0;
                int i;
                for (i = 0; i < SHA_DIGEST_LENGTH; i++){
                    len += sprintf(livehash+len, "%02x", temphash[i]);
                }
                //compare live hash to hash in client manifest
                if (strcmp(livehash, hash) != 0){
                    //Client and server manifest match but live hash is different than client manifest
                    char livehashcopy[NAME_MAX];
                    strcpy(livehashcopy, livehash);
                    bzero(livehash, sizeof(livehash));
                    strcpy(livehash, "M ");
                    strcat(livehash, file);
                    printf("%s\n", livehash);
                    strcat(livehash, " ");
                    strcat(livehash, livehashcopy);
                    strcat(livehash, " ");
                    char* temp = int_to_string(++version, false);
                    strcat(livehash, temp);
                    strcat(livehash, "\n");
                    free(temp);
                    fwrite(livehash, 1, strlen(livehash), commit);
                    increment_version_number(version, project, file, livehashcopy);
                    strcpy(ptr->project, "done");
                    return;
                }
            } else if(ptr->version >= version){
                printf("The server file is a higher version but doesn't match client hash, please update\n");
                write(socket, "done:done", 10);
                char buffer[PATH_MAX];
                strcpy(buffer, ".");
                append_file_path(buffer, project);
                append_file_path(buffer, ".Commit");
                fclose(commit);
                close(socket);
                remove(buffer);
                exit(1);
            }
            found = true;
            strcpy(ptr->project, "done");
        } else if (strlen(project) == 0){ //We reached the end of the client manifest, any nodes in the server linked list without a done as their project name are files not in client manifest.
            while (head != NULL){
                if (strcmp(head->project, "done") != 0 && strlen(head->file) != 0){
                    char buffer[PATH_MAX];
                    bzero(buffer, sizeof(buffer));
                    strcpy(buffer, "D ");
                    strcat(buffer, head->file);
                    printf("%s\n", buffer);
                    strcat(buffer, " ");
                    strcat(buffer, head->hash);
                    strcat(buffer, " ");
                    char* temp = int_to_string(head->version, false);
                    strcat(buffer, temp);
                    free(temp);
                    strcat(buffer, "\n");
                    fwrite(buffer, 1, strlen(buffer), commit);
                    //I am not incrementing version value because the file DNE on client manifest
                }
                head = head->next;
            }
            return;
        }
        ptr = ptr->next;
    }
    //If we searched through the server manifest and did not find the file that means we need to add a D to update
    if (!found){
        char buffer[PATH_MAX];
        bzero(buffer, sizeof(buffer));
        strcpy(buffer, "A ");
        strcat(buffer, file);
        printf("%s\n", buffer);
        strcat(buffer, " ");
        strcat(buffer, hash); //Is this suppose to be the server hash or the client hash?
        strcat(buffer, " ");
        char* temp = int_to_string(++version, false);
        strcat(buffer, temp);
        free(temp);
        strcat(buffer, "\n");
        fwrite(buffer, 1, strlen(buffer), commit);
        increment_version_number(version, project, file, hash);
    }
}

void check_host_name(int hostname) { 
   if (hostname == -1) {
      perror("gethostname");
      exit(1);
   }
}
void check_host_entry(struct hostent * hostentry) { 
   if (hostentry == NULL) {
      perror("gethostbyname");
      exit(1);
   }
}
void IP_formatter(char *IPbuffer) { 
   if (NULL == IPbuffer) {
      perror("inet_ntoa");
      exit(1);
   }
}

char* getLocalHostIP(){
    char host[256];
    char *IP;
    struct hostent *host_entry;
    int hostname;
    hostname = gethostname(host, sizeof(host));
    check_host_name(hostname);
    host_entry = gethostbyname(host); 
    check_host_entry(host_entry);
    IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    //Convert into IP string
    return IP;
}


void testcommit(int socket, char* project){
    //Error checking
    char buffer[PATH_MAX];
    char c;
    int n = 0;
    bzero(buffer, sizeof(buffer));
    read(socket, buffer, sizeof(buffer));
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project or project manifest on server\n");
        write(socket, "done:done", 9);
        close(socket);
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("success in finding project on server\n");
    }
    //Error checking for client manifest 
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    if (!file_exists(buffer)){
        printf("no manifest on client\n");
        write(socket, "done:done", 9);
        close(socket);
        exit(1);
    }
    delete_last_file_path(buffer, ".Manifest");
    append_file_path(buffer, ".Update");
    //Error check to see if there is a non-empty .Update file
    if (!is_file_empty(buffer)){
        printf("non-empty .Update file in client\n");
        write(socket, "done:done", 9);
        close(socket);
        exit(1);
    }
    delete_last_file_path(buffer, ".Update");
    append_file_path(buffer, ".Conflict");
    if(file_exists(buffer)){
        printf("Client has .Conflict file\n");
        write(socket, "done:done", 9);
        close(socket);
        exit(1);
    }
    bool sendfile = false;
    bzero(buffer, sizeof(buffer));
    while (true){
        while(read(socket, &c, 1) != 0 && c != ':'){
            buffer[n++] = c;
        } 
        if (strcmp(buffer, "done") == 0) break;
        else if (strcmp(buffer, "sendfile") == 0) {
            manifestNode* server = NULL;
            manifestNode* client = NULL;
            get_client_and_server_manifests(&server, &client, socket, project);
            //See if the client and server manifest versions match
            printf("server\n");
            print_manifest_List(server);
            printf("client\n");
            print_manifest_List(client);
            manifestNode* ptr_s = server;
            manifestNode* ptr_c = client;
            while (ptr_s->next != NULL){
                ptr_s = ptr_s->next;
            }
            while (ptr_c->next != NULL){
                ptr_c = ptr_c->next;
            }
            if (ptr_c->version != ptr_s->version){
                //Since the versions don't match we can tell the client to update
                printf("The client and server manifest verisons do not match, please run update\n");
                write(socket, "done:done", 9);
                close(socket);
                exit(1);
            }
            int version = ptr_c->version + 1;
            char* version_s = int_to_string(version, true);
            version_s[strlen(version_s) - 1] = '\n';
            update_manifest_version(project, version_s);
            ptr_c = client;
            bzero(buffer, sizeof(buffer));
            strcpy(buffer, ".");
            append_file_path(buffer, project);
            append_file_path(buffer, ".Commit");
            FILE* commit = fopen(buffer, "w+");
            fputs(version_s, commit);
            free(version_s);
            /*
            delete_last_file_path(buffer, ".Commit");
            append_file_path(buffer, ".Conflict");
            FILE* conflict = fopen(buffer, "w");
            */
            while (ptr_c != NULL){
                manifest_case_selection_commit(ptr_c->version, ptr_c->project, ptr_c->file, ptr_c->hash, server, commit, socket);
                ptr_c = ptr_c->next;
            }
            //Check if commit file is empty, if so delete it
            change_update(commit, buffer);
            fclose(commit);
            char host[PATH_MAX];
            bzero(host, sizeof(host));
            strcpy(host, "host:");
            strcat(host, getLocalHostIP());
            strcat(host, ":");
            write(socket, host, strlen(host));
            write_bytes_to_socket(buffer, socket, true);
            write(socket, "done:", 5);
        } else if (strcmp(buffer, "success") == 0){
            printf("Server successfully saved the active commit\n");
        }

        bzero(buffer, sizeof(buffer));
        n = 0;
    }


}

void update_manifest_version(char* project, char* newversion){
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");

    long length = get_file_length(buffer);
    char* oldstream = malloc(sizeof(char) * (length+1));
    bzero(oldstream, sizeof(char) * (length+1));

    FILE* manifestFD = fopen(buffer, "r");
    if (manifestFD == NULL){
        printf("error in opening manifest\n");
        return;
    }
    fread(oldstream, 1, length, manifestFD);

    char* token = strtok(oldstream, "\n");
    char* newstream = malloc(sizeof(char) * (length+1));
    bzero(newstream, sizeof(char) * (length+1));

    strcat(newstream, newversion);
    strcat(newstream, "\n");

    while((token = strtok(NULL, "\n")) != NULL){
        strcat(newstream, token);
        strcat(newstream, "\n");
    }
    FILE* newManifestFD = fopen(buffer, "w");
    if (newManifestFD == NULL){
        printf("error in opening new manifest\n");
        return;
    }
    int newSize = strlen(newstream);
    fwrite(newstream, 1, newSize, newManifestFD);
    fclose(manifestFD);
    fclose(newManifestFD);


}

void testupgrade(int socket, char* project){
    //Error checking
    char buffer[PATH_MAX];
    read(socket, buffer, sizeof(buffer));
    //Checks if project in server
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project on server\n");
        write(socket, "done:done", 9);
        close(socket);
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("success in finding project on server\n");
    }
    //Checks if .Update file exists
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Conflict");
    if (file_exists(buffer)){
        printf("Conflict file exists in client\n");
        write(socket, "done:done", 9);
        close(socket);
        exit(1);
    }
    //Checks if .Conflict file exists
    delete_last_file_path(buffer, ".Conflict");
    append_file_path(buffer, ".Update");

    FILE* update = fopen(buffer, "r");
    if (update == NULL){
        printf("No .Update file in client\n");
        write(socket, "done:done", 9);
        close(socket);
        exit(1); 
    }
    long length = get_file_length(buffer);
    if (length == 0){
        printf("The .Update file is empty and the project is Up to Date\n");
        remove(buffer);
        write(socket, "done:done", 9);
        close(socket);
        exit(1); 
    }
    bzero(buffer, sizeof(buffer));

    //Update the client manifest version
    fgets(buffer, length, update);
    char* delim_special = "\n";
    char* token_special = strtok(buffer, delim_special);
    update_manifest_version(project, token_special);
    bzero(buffer, sizeof(buffer));

    while (fgets(buffer, length, update) != NULL){
        char* delim = " \n";
        char* token = strtok(buffer, delim);
        //Adds M, A, or D
        char mode = token[0];
        //Adds the filepath
        token = strtok(NULL, delim);
        char filepath[PATH_MAX];
        strcpy(filepath, token);
        //Adds the hash
        token = strtok(NULL, delim);
        char hash[PATH_MAX];
        strcpy(hash, token);
        token = strtok(NULL, delim);
        char version_s[PATH_MAX];
        strcpy(version_s, token);

        if (mode == 'A' || mode == 'M'){
            char sendrequest[PATH_MAX];
            strcpy(sendrequest, "requestfile:");
            strcat(sendrequest, filepath);
            strcat(sendrequest, ":");
            write(socket, sendrequest, strlen(sendrequest));
            bzero(sendrequest, sizeof(buffer));
            char c;
            int n = 0;
            while (read(socket, &c, 1) != 0 && c != ':'){
                sendrequest[n++] = c;
            }
            if (strcmp(sendrequest, "sendfile") == 0){
                bzero(sendrequest, sizeof(buffer));
                n = 0;
                while (read(socket, &c, 1) != 0 && c != ':'){
                    sendrequest[n++] = c;
                }
                long file_size = strtol(sendrequest, NULL, 0);
                FILE* add = fopen(filepath, "w");
                if (add == NULL){
                    printf("something went wrong with opening adding file %s\n");
                    write(socket, "done:done", 9);
                    close(socket);
                    exit(1);
                }
                while (file_size > 0 && read(socket, &c, 1) != 0){
                    fwrite(&c, 1, 1, add);
                    file_size--;
                }
                fclose(add);
            }
            if (mode == 'A'){
                testadd(project, filepath);
            }
            int version = atoi(version_s);
            increment_version_number(version, project, filepath, hash);
        } else if (mode == 'D'){
            delete_line_from_manifest(project, filepath, hash);
        } 
        bzero(buffer, sizeof(buffer));
    }
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Update");
    remove(buffer);
    write(socket, "done:", 5);
}

void testpush(int socket, char* project){
    char buffer[PATH_MAX];
    read(socket, buffer, sizeof(buffer));
    //Checks if project in server
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project on server\n");
        write(socket, "done:done", 9);
        close(socket);
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("success in finding project on server\n");
    }
    //Check if client has .Commit file
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Commit");
    FILE* commit = fopen(buffer, "r");
    if (commit == NULL){
        printf("failure in finding .Commit on client\n");
        write(socket, "done:done", 9);
        close(socket);
        exit(1);
    }
    //Send host to server so it can check if it has a .Commit file coressponding to us
    char host[NAME_MAX];
    bzero(host, sizeof(host));
    strcpy(host, "host:");
    strcat(host, getLocalHostIP());
    strcat(host, ":");
    write(socket, host, strlen(host));
    write_bytes_to_socket(buffer, socket, true);
    char c;
    int n = 0;
    bzero(buffer, sizeof(buffer));
    while (true){
        while(read(socket, &c, 1) != 0 && c != ':'){
            buffer[n++] = c;
        }

        if (strcmp(buffer, "done") == 0) break;
        else if (strcmp(buffer, "fail") == 0) {
            printf("Server failed\n");
            bzero(buffer, sizeof(buffer));
            strcpy(buffer, ".");
            append_file_path(buffer, project);
            append_file_path(buffer, ".Commit");
            write(socket, "done", 5);
            remove(buffer);
            break;
        } else if (strcmp(buffer, "requestfile") == 0){
            bzero(buffer, sizeof(buffer));
            n = 0;
            while (read(socket, &c, 1) != 0 && c != ':'){
                buffer[n++] = c;
            }
            //buffer holds the name of the file we need to send
            write_bytes_to_socket(buffer, socket, true); //sends the current version of the file
        } else if (strcmp(buffer, "success") == 0){
            //We can delete our local .Commit file
            printf("server succeeded\n");
            bzero(buffer, sizeof(buffer));
            strcpy(buffer, ".");
            append_file_path(buffer, project);
            append_file_path(buffer, ".Commit");
            remove(buffer);
            break;
        }

        bzero(buffer, sizeof(buffer));
        n = 0;
    }
    fclose(commit);
}

void testdestroy(int socket, char* project){
    char buffer[PATH_MAX];
    read(socket, buffer, sizeof(buffer));
    //Checks if project in server
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project on server\n");
        write(socket, "done", 5);
        close(socket);
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("success in finding project on server\n");
    }
    //Send host information
    char host[NAME_MAX];
    bzero(host, sizeof(host));
    strcpy(host, "host:");
    strcat(host, getLocalHostIP());
    strcat(host, ":");
    write(socket, host, strlen(host));

    int n = 0;
    char c;
    bzero(buffer, sizeof(buffer));
    while (true){
        while (read(socket, &c, 1) != 0 && c != ':'){
            buffer[n++] = c;
        }

        if (strcmp(buffer, "success") == 0){
            printf("The server has successfuly destroyed the project\n");
            break;
        } else if (strcmp(buffer, "fail") == 0){
            printf("The server has not successfuly destroyed the project\n");
            break;
        }
        bzero(buffer, sizeof(buffer));
        n = 0;
    }
}

void testcurrentversion(int socket, char* project){
    char buffer[PATH_MAX];
    read(socket, buffer, sizeof(buffer));
    //Checks if project in server
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project on server\n");
        write(socket, "done", 5);
        close(socket);
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("success in finding project on server\n");
    }

    int n = 0;
    char c;
    bzero(buffer, sizeof(buffer));
    while(true){
        while(read(socket, &c, 1) != 0 && c != ':'){
            buffer[n++] = c;
        }

        if (strcmp(buffer, "done") == 0) break;
        else if (strcmp(buffer, "fail") == 0){
            printf("No manifest inside %s\n", project);
            return;
        }
        printf("%s", buffer);
        bzero(buffer, sizeof(buffer));
        n = 0;
    }
}

void testrollback(int socket, char* project, char* version){
    char buffer[PATH_MAX];
    read(socket, buffer, sizeof(buffer));
    //Checks if project in server
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project on server\n");
        write(socket, "done", 5);
        close(socket);
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("success in finding project on server\n");
    }
    //send which 

    int n = 0;
    char c;
    bzero(buffer, sizeof(buffer));
    while(true){
        while(read(socket, &c, 1) != 0 && c != ':'){
            buffer[n++] = c;
        }

        if (strcmp(buffer, "done") == 0) break;
        else if (strcmp(buffer, "fail") == 0){
            printf("rollback failed\n", project);
            return;
        }
    
        bzero(buffer, sizeof(buffer));
        n = 0;
    }
}

void testhistory(int socket, char* project){
    char buffer[PATH_MAX];
    read(socket, buffer, sizeof(buffer));
    //Checks if project in server
    if (strcmp(buffer, "fail") == 0){
        printf("failure in finding project on server\n");
        write(socket, "done", 5);
        close(socket);
        exit(1);
    } else if (strcmp(buffer, "success") == 0){
        printf("success in finding project on server\n");
    }

    int n = 0;
    char c;
    bzero(buffer, sizeof(buffer));
    while(true){
        while(read(socket, &c, 1) != 0 && c != ':'){
            buffer[n++] = c;
        }

        if (strcmp(buffer, "done") == 0) break;
        else if (strcmp(buffer, "fail") == 0){
            printf("No manifest inside %s\n", project);
            return;
        }
        printf("%s", buffer);
        bzero(buffer, sizeof(buffer));
        n = 0;
    }
}

void handle_connection(int socket, char** argv){
    char command[NAME_MAX];
    char project_name[NAME_MAX];
    char arg[NAME_MAX];
    bzero(command, sizeof(command));
    bzero(project_name, sizeof(project_name));
    bzero(arg, sizeof(arg));
    strcpy(command, argv[1]);
    strcpy(project_name, argv[2]);
    write(socket, command, sizeof(command));
    write(socket, project_name, sizeof(project_name));

    if (strcmp(argv[1], "checkout") == 0) {
        //Check if project exists on client side
        if (!project_exists_on_client(argv[2])){
            testcheckout(socket, project_name);
        }
    } else if (strcmp(argv[1], "update") == 0){
        if (project_exists_on_client(argv[2])){
            testupdate(socket, project_name);
        }
    } else if (strcmp(argv[1], "upgrade") == 0){
        if (project_exists_on_client(argv[2])){
            testupgrade(socket, project_name);
        } else {
            write(socket, "done:", 5);
        }
    } else if (strcmp(argv[1], "commit") == 0){
        if (project_exists_on_client(argv[2])){
            testcommit(socket, project_name);
        } else {
            write(socket, "done:", 5);
        }
    } else if (strcmp(argv[1], "push") == 0){
        if (project_exists_on_client(argv[2])){
            testpush(socket, project_name);
        } else {
            write(socket, "done:", 5);
        }
    } else if (strcmp(argv[1], "create") == 0){
        if (!project_exists_on_client(argv[2])){
            testcreate(socket, project_name);
        }
    } else if (strcmp(argv[1], "destroy") == 0){
        testdestroy(socket, project_name);
    } else if (strcmp(argv[1], "currentversion") == 0){
        testcurrentversion(socket, project_name);
    } else if (strcmp(argv[1], "history") == 0){
        testhistory(socket, project_name);
    } else if (strcmp(argv[1], "rollback") == 0){
        strcpy(arg, argv[3]);
        write(socket, arg, sizeof(arg));
        testrollback(socket, project_name, arg);
    } /*else if (strcmp(argv[1], "done") == 0){
        strcpy(command, argv[1]);
        write(socket, command, sizeof(command));
    }*/
    strcpy(command, "done");
    write(socket, command, sizeof(command));
    close(socket);
}

void testadd(char* project, char* filename){
    //Error checking
    if (!project_exists_on_client(project)){
        printf("Project does not exist on client\n");
        return;
    }
    //Check if file and Manifest exists
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    if (file_exists(filename) && file_exists(buffer)){
        //Create hash for file
        unsigned char hash[SHA_DIGEST_LENGTH];
        bzero(hash, sizeof(hash));
        calculate_and_update_hash(filename, hash);
        //Add file to manifest
        FILE* fp = fopen(buffer, "a");
        bzero(buffer, sizeof(buffer));
        strcat(buffer, "1 ");
        strcat(buffer, project);
        strcat(buffer, " ");
        strcat(buffer, filename);
        strcat(buffer, " ");
        int len = strlen(buffer);
        int i;
        for (i = 0; i < SHA_DIGEST_LENGTH; i++){
            len += sprintf(buffer+len, "%02x", hash[i]);
        }
        //strcat(buffer, hash);
        strcat(buffer, "\n");
        fprintf(fp, buffer);
        fclose(fp);
    }
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

void testremove(char* project, char* file){
    if (!project_exists_on_client(project)){
        printf("Project does not exist on client\n");
        return;
    }
    //Check if file and Manifest exists
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    if (file_exists(buffer)){
        long length = get_file_length(buffer);
        char* oldstream = malloc(sizeof(char) * (length+1));
        bzero(oldstream, sizeof(char) * (length+1));
        FILE* manifestFD = fopen(buffer, "r");
        if (manifestFD == NULL){
            printf("error in opening manifest\n");
            return;
        }
        fread(oldstream, 1, length, manifestFD);
        char* token = strtok(oldstream, "\n");
        char* newstream = malloc(sizeof(char) * (length+1));
        bzero(newstream, sizeof(char) * (length+1));
        strcat(newstream, token);
        strcat(newstream, "\n");
        
        while((token = strtok(NULL, "\n")) != NULL){
            if(strstr(token, project) == NULL || strstr(token, file) == NULL ){
                strcat(newstream, token);
                strcat(newstream, "\n");
            }
        }

        FILE* newManifestFD = fopen(buffer, "w");
        if (newManifestFD == NULL){
            printf("error in opening new manifest\n");
            return;
        }
        int newSize = strlen(newstream);
        fwrite(newstream, 1, newSize, newManifestFD);
        fclose(manifestFD);
        fclose(newManifestFD);
    }
}

bool non_socket_commands(char** argv){
    char project_name[NAME_MAX];
    char arg[NAME_MAX];
    bzero(project_name, sizeof(project_name));
    bzero(arg, sizeof(arg));
    if (strcmp(argv[1], "add") == 0){
        strcpy(project_name, argv[2]);
        strcpy(arg, argv[3]);
        testadd(project_name, arg);
        return true;
    } else if (strcmp(argv[1], "remove") == 0){
        strcpy(project_name, argv[2]);
        strcpy(arg, argv[3]);
        testremove(project_name, arg);
        return true;
    }
    return false;
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
    } else if(!non_socket_commands(argv)) {
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
