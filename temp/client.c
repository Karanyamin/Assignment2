
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

long get_file_length(char* file);

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

bool file_exists(char* filename){
    int fd = open(filename, O_RDWR);
    if (fd == -1){
        printf("File %s does not exist\n", filename);
        return false;
    }
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
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "D ");
    strcat(buffer, file);
    printf("%s\n", buffer);
    strcat(buffer, " ");
    strcat(buffer, hash);
    strcat(buffer, "\n");
    fwrite(buffer, 1, strlen(buffer), update);
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
            //sendFile(socket);
            char* eptr;
            bzero(buffer, sizeof(buffer));
            n = 0;
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
            manifestNode * serverhead = NULL;
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
                    serverhead = insert_To_Manifest_List(serverhead, version, projectName, filepath, hash);
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
            manifestNode* clienthead = NULL;
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
                    clienthead = insert_To_Manifest_List(clienthead, version, projectName, filepath, hash);
                    bzero(buffer, sizeof(buffer));
                    n = 0;
                } else {
                    buffer[n++] = c;
                }
                number_of_bytes_SM--;
            }
            close(fd);
            printf("server manifest\n");
            print_manifest_List(serverhead);
            printf("client manifest\n");
            print_manifest_List(clienthead);
            //Compare both manifest linked lists
            //Case 1 both manifest versions match
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
                ptr_c = clienthead;
                bzero(buffer, sizeof(buffer));
                strcpy(buffer, ".");
                append_file_path(buffer, project);
                append_file_path(buffer, ".Update");
                FILE* update = fopen(buffer, "w");
                delete_last_file_path(buffer, ".Update");
                append_file_path(buffer, ".Conflict");
                FILE* conflict = fopen(buffer, "w");
                while (ptr_c != NULL){
                    manifest_case_selection(ptr_c->version, ptr_c->project, ptr_c->file, ptr_c->hash, serverhead, update, conflict);
                    ptr_c = ptr_c->next;
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
        return;
    } else if (strcmp(argv[1], "commit") == 0){
        return;
    } else if (strcmp(argv[1], "push") == 0){
        return;
    } else if (strcmp(argv[1], "create") == 0){
        if (!project_exists_on_client(argv[2])){
            testcreate(socket, project_name);
        }
    } else if (strcmp(argv[1], "destory") == 0){
        return;
    } else if (strcmp(argv[1], "currentversion") == 0){
        return;
    } else if (strcmp(argv[1], "history") == 0){
        return;
    } else if (strcmp(argv[1], "rollback") == 0){
        return;
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
        char* oldstream = malloc(sizeof(char) * length);
        bzero(oldstream, sizeof(char) * length);
        FILE* manifestFD = fopen(buffer, "r");
        if (manifestFD == NULL){
            printf("error in opening manifest\n");
            return;
        }
        fread(oldstream, 1, length, manifestFD);
        char* token = strtok(oldstream, "\n");
        char* newstream = malloc(sizeof(char) * length);
        bzero(newstream, sizeof(char) * length);
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
