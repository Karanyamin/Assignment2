
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
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>

long get_file_length(char* file);
char* int_to_string(long num, bool delim);
bool is_IPv4_address(const char* s);
void append_file_path(char* path, char* nextDirectoryName);
void delete_last_file_path(char* path, char* nextDirectoryName);
void replace_str(char* str, char* replacefrom, char* replaceto);

char* append;

typedef struct mutex_node {
    char* project;
    pthread_mutex_t mutex;
    struct mutex_node * next;
} mutex_node;

mutex_node * head = NULL;

pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

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

bool included_in_manifest(char* project, char* file){
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, append); //NEW
    //strcpy(buffer, ".");  //OLD
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    if (strcmp(file, buffer) == 0) return true;
    long length = get_file_length(buffer);
    FILE* manifestFD = fopen(buffer, "r");
    if (manifestFD == NULL){
        printf("error in opening manifest\n");
        return false;
    }
   
    bzero(buffer, sizeof(buffer));
    fgets(buffer, ++length, manifestFD); //Throwing away first link
    bzero(buffer, sizeof(buffer));

    while (fgets(buffer, length, manifestFD) != NULL){
        bool p = false, f = false; //Each bool corresponds to the function parameters
        char* delim = " \n";
        char* token = strtok(buffer, delim); //token holds version number

        token = strtok(NULL, delim); //token holds project name
        if (strcmp(token, project) == 0) p = true;
        token = strtok(NULL, delim); //token golds project filepath
        if (strcmp(token, file) == 0) f = true;

        if (p && f){
            fclose(manifestFD);
            return true;
        } 
    }   
    
    fclose(manifestFD);
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
        close(fd);
        return;
    }
    //prints out the hash in hexadecimal
    printf("The hash for the file [%s] is [", filepath);
    int i;
    for (i = 0; i < SHA_DIGEST_LENGTH; i++){
        printf("%02x", hash[i]);
    }
    printf("]\n");
    close(fd);
}

bool project_exists_on_server(char* name){
    if (strcmp(name, "ignore") == 0) return true;

    DIR* directory = opendir(append); //NEW
    //DIR* directory = opendir(".");  //OLD
    if (directory == NULL){
        printf("error on checking if project %s exists on server\n", name);
        return false;
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
    } else if (is_IPv4_address(name)){
        return true;
    } else if (strcmp(name, ".history") == 0){
        return true;
    } else {
        return false;
    }
    
}

int recursively_destory_directory(char* path){

    DIR* dir = opendir(path);
    struct dirent * handle;

    if (dir == NULL){
        printf("Error opening directory with path name: [%s] in recursive Behavior\n", path);
        return -1;
    }

    while ((handle = readdir(dir)) != NULL){
        if (strcmp(handle->d_name, ".") != 0 && strcmp(handle->d_name, "..") != 0){
            if (handle->d_type == DT_DIR){
                append_file_path(path, handle->d_name);
                if (recursively_destory_directory(path) == -1) return -1;
                if (rmdir(path) == -1) return -1; //Deletes the empty directory
                delete_last_file_path(path, handle->d_name);
            } else if (handle->d_type == DT_REG){
                append_file_path(path, handle->d_name);
                printf("Deleting files [%s]\n", path);
                if (remove(path) != 0){
                    printf("Couldn't remove file [%s]\n", path);
                    return -1;
                }
                delete_last_file_path(path, handle->d_name);
            }
        }
    }
    closedir(dir);
    return 0;
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
        fclose(fp);
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

int write_bytes_of_all_files(char* path, int socket, char* project){

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
                if (write_bytes_of_all_files(path, socket, project) == 0){
                    return 0;
                }
                delete_last_file_path(path, handle->d_name);
            } else if (handle->d_type == DT_REG){
                append_file_path(path, handle->d_name);
                if (included_in_manifest(project, path)){
                    printf("Scanning this file %s for writing bytes to socket\n", path);
                    if (write_bytes_to_socket(path, socket, false) == 0){
                        return 0;
                    }
                }
                
                delete_last_file_path(path, handle->d_name);
            }
        }
    }
    closedir(dir);
    return 1;
}

int get_number_of_files_in_project(char* project, int* size){
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, append); //NEW
    //strcpy(buffer, ".");  //OLD
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    long length = get_file_length(buffer);
    FILE* manifestFD = fopen(buffer, "r");
    if (manifestFD == NULL){
        printf("error in opening manifest\n");
        return 0;
    }
   
    bzero(buffer, sizeof(buffer));
    char c;
    while ((c = fgetc(manifestFD)) != EOF){
        if (c == '\n')
            (*size)++;
    }   
    
    fclose(manifestFD);
    return 1;
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

int write_all_files_to_socket_in_sequence(char* path, int socket, char* project){

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
                if (write_all_files_to_socket_in_sequence(path, socket, project) == 0){
                    return 0;
                }
                delete_last_file_path(path, handle->d_name);
            } else if (handle->d_type == DT_REG){
                append_file_path(path, handle->d_name);
                if (included_in_manifest(project, path)){
                    printf("Scanning this file %s for appending file to socket\n", path);
                    char* length_of_file = int_to_string(strlen(path), true);
                    if (length_of_file == NULL) return 0;
                    write(socket, length_of_file, strlen(length_of_file));
                    write(socket, path, strlen(path));
                    char* bytes_in_file = int_to_string(get_file_length(path), true);
                    if (bytes_in_file == NULL) return 0;
                    write(socket, bytes_in_file, strlen(bytes_in_file));
                    free(length_of_file);
                    free(bytes_in_file);
                }
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

void checkout(char* project, int socket){
    printf("In checkout\n");
    //Error checking
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    if (!project_exists_on_server(project)){
        printf("%s does not exit in the server\n", project);
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }
    bzero(path, sizeof(path));
    strcpy(path, append); //NEW
    //strcpy(path, ".");  //OLD
    append_file_path(path, project);
    append_file_path(path, ".Manifest");
    if (!file_exists(path)){
        printf("No manifest in project\n");
        write(socket, "fail:", 5);
        return;
    }
    //writing send command to socket
    write(socket, "sendfile:", 9);
    //Finding out how many files to send
    int number_of_files = 0;
    bzero(path, sizeof(path));
    strcpy(path, append);
    append_file_path(path, project);
    if(get_number_of_files_in_project(project, &number_of_files) == 0){
        printf("Error in getting number of files in project\n");
        return;
    }
    char* number_of_files_s = int_to_string(number_of_files, true);
    if (number_of_files_s == NULL) return;
    write(socket, number_of_files_s, strlen(number_of_files_s));
    free(number_of_files_s);
    //write file path names in sequence
    bzero(path, sizeof(path));

    strcpy(path, append);
    append_file_path(path, project);
    if(write_all_files_to_socket_in_sequence(path, socket, project) == 0){
        printf("Error in writing all filepath names to socket\n");
        return;
    }
    //write all bytes of all files in order
    bzero(path, sizeof(path));
    strcpy(path, append);
    append_file_path(path, project);
    write_bytes_of_all_files(path, socket, project);
    write(socket, ":done:", 6);
}

long get_file_length(char* file){
    FILE* fd = fopen(file, "r");
    if (fd == NULL){
        printf("Error in get_file_length trying to open file\n");
        return -1;
    }
    if (fseek(fd, 0L, SEEK_END) != 0){
        printf("Error in get_file_length trying to seek file\n");
        fclose(fd);
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
    bzero(path, sizeof(path));
    strcpy(path, append);
    append_file_path(path, project_name);
    if (mkdir(path, 0777) == -1){
        printf("The server failed to make the directory\n");
        return;
    }
    bzero(path, sizeof(path));
    strcpy(path, append); //NEW
    //strcpy(path, "."); //OLD
    append_file_path(path, project_name);
    append_file_path(path, ".Manifest");
    //Creates the .Manifest file and puts it in the project
    int fd = open(path, O_WRONLY | O_CREAT, 00600);
    if (fd == -1){
        printf("The server failed to create the Manifest in the directory\n");
        bzero(path, sizeof(path));
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    }
    delete_last_file_path(path, ".Manifest");
    append_file_path(path, ".history");
    int historyfd = open(path, O_WRONLY | O_CREAT, 00600);
    if (historyfd == -1){
        printf("The server failed to create the .history file in project %s\n", project_name);
        bzero(path, sizeof(path));
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        close(fd);
        return;
    }
    write(fd, "1\n", 2);
    close(fd);
    close(historyfd);
    bzero(path, sizeof(path));
    strcpy(path, "success");
    write(socket, path, sizeof(path));
    checkout(project_name, socket);
}

void update(char* project, int socket){
    //Error checking to see if project exists on server
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    if (!project_exists_on_server(project)){
        printf("Project does not exist on server\n");
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }
    bzero(path, sizeof(path));
    strcpy(path, append); //NEW
    //strcpy(path, ".");  //OLD
    append_file_path(path, project);
    append_file_path(path, ".Manifest");
    if (!file_exists(path)){
        printf("No manifest in project\n");
        write(socket, "fail:", 5);
        return;
    }
    char* length_of_manifest = int_to_string(get_file_length(path), true);
    if (length_of_manifest == NULL){
        printf("error trying to get length of manifest\n");
        write(socket, "fail:", 5);
        return;
    }
    write(socket, "sendfile:", 9);
    write(socket, length_of_manifest, strlen(length_of_manifest));
    free(length_of_manifest);
    if (write_bytes_to_socket(path, socket, false) == 0){
        printf("Error in sending manifest to client\n");
        return;
    }
    write(socket, "done:", 5);
}

void commit(char* project, int socket){
    int n = 0;
    char c;
    printf("I am in commit\n");
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    strcpy(path, append); //NEW
    //strcpy(path, ".");  //OLD
    append_file_path(path, project);
    append_file_path(path, ".Manifest");
    if (!project_exists_on_server(project) || !file_exists(path)){
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }

    bzero(path, sizeof(path));
    while (true){
        while (read(socket, &c, 1) != 0 && c != ':'){
            path[n++] = c;
        }

        if (strcmp(path, "fail") == 0){
            printf("Client error failed\n");
            return;
        } else if (strcmp(path, "success") == 0){
            break;
        }
        bzero(path, sizeof(path));
        n = 0;
    }

    bzero(path, sizeof(path));
    strcpy(path, append); //NEW
    //strcpy(path, ".");  //OLD
    append_file_path(path, project);
    append_file_path(path, ".Manifest");
    char* length_of_manifest = int_to_string(get_file_length(path), true);
    if (length_of_manifest == NULL){
        printf("Error in finding length of manifest\n");
        return;
    }
    write(socket, "sendfile:", 9);
    write(socket, length_of_manifest, strlen(length_of_manifest));
    if (write_bytes_to_socket(path, socket, false) == 0){
        printf("Error in sending manifest to client\n");
        return;
    }
    bzero(path, sizeof(path));
    char host[PATH_MAX];
    bzero(host, sizeof(host));
    n = 0;
    while (true){
        while (read(socket, &c, 1) != 0 && c != ':'){
            path[n++] = c;
        }
        
        if (strcmp(path, "sendfile") == 0){
            bzero(path, sizeof(path));
            FILE* fp = fopen(host, "w");

            n = 0;
            while (read(socket, &c, 1) != 0 && c != ':'){
                path[n++] = c;
            }
            long length = strtol(path, NULL, 0);
            
            while (length > 0 && read(socket, &c, 1) != 0){
                fwrite(&c, 1, 1, fp);
                length--;
            }
            write(socket, "success:", 8);
            fclose(fp);
        } else if (strcmp(path, "done") == 0){
            break;
        } else if (strcmp(path, "host") == 0){
            bzero(path, sizeof(path));
            n = 0;
            while (read(socket, &c, 1) != 0 && c != ':'){
                path[n++] = c;
            }
            strcpy(host, append);
            append_file_path(host, project);
            strcat(host, "_");
            strcat(host, path);
        }
        bzero(path, sizeof(path));
        n = 0;
    }
    write(socket, "done:", 5);
}

void upgrade(char* project, int socket){
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    strcpy(path, append); //NEW
    //strcpy(path, "."); //OLD
    append_file_path(path, project);
    append_file_path(path, ".Manifest");

    if (!project_exists_on_server(project) || !file_exists(path)){
        printf("Project or Manifest does not exist on server\n");
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }
    int n = 0;
    char c;
    bzero(path, sizeof(path));
    while (true){
        bzero(path, sizeof(path));
        n = 0;
        while (read(socket, &c, 1) != 0 && c != ':'){
            path[n++] = c;
        }
        
        if (strcmp(path, "requestfile") == 0){
            bzero(path, sizeof(path));
            n = 0;
            while (read(socket, &c, 1) != 0 && c != ':'){
                path[n++] = c;
            }
            //path contains the file requested by client, check if it exists
            if (!file_exists(path)){
                printf("file %s does not exist in project %s\n", path, project);
                write(socket, "fail:", 5);
                bzero(path, sizeof(path));
                n = 0;
                continue;
            }
            write_bytes_to_socket(path, socket, true);
        } else if (strcmp(path, "done") == 0){
            break;
        }
        bzero(path, sizeof(path));
        n = 0;
    }

}

bool file_same(FILE* a, FILE* b){
    rewind(a);
    rewind(b);
    char c, d;
    do {
        c = fgetc(a);
        d = fgetc(b);

        if (c != d) return false;

    } while (c != EOF && d != EOF);

    if (c == EOF && d == EOF){ //If both files eached the end and every char was the same, return true. If one file is longer return false
        return true;
    } else {
        return false;
    }
}

bool is_IPv4_address(const char* s){
    int len = strlen(s);

    if (len < 7 || len > 15)
        return false;

    char tail[16];
    tail[0] = 0;

    unsigned int d[4];
    int c = sscanf(s, "%3u.%3u.%3u.%3u%s", &d[0], &d[1], &d[2], &d[3], tail);

    if (c != 4 || tail[0])
        return false;

    int i;
    for (i = 0; i < 4; i++)
        if (d[i] > 255)
            return false;

    return true;
}

void expire_all_other_commits(char* host, char* project){
    //DIR* dir = opendir("."); //OLD
    DIR* dir = opendir(append); //NEW
    struct dirent* handle;

    while ((handle = readdir(dir)) != NULL){
        char buffer[NAME_MAX];
        strcpy(buffer, handle->d_name);
        char* delim = "_";
        char* token = strtok(buffer, delim);
        char project_temp[NAME_MAX];
        char host_temp[NAME_MAX];
        bzero(project_temp , sizeof(project_temp));
        bzero(host_temp , sizeof(host_temp));
        if (token != NULL)
            strcpy(project_temp, token);

        token = strtok(NULL, delim);
        if (token != NULL)
            strcpy(host_temp, token);
        
        if (is_IPv4_address(host_temp) && strcmp(project_temp, project) == 0 && strcmp(handle->d_name, host) != 0){
            remove(handle->d_name);
        } 
    }
    closedir(dir);
}

void duplicate(char* a, char* b){
    FILE* old = fopen(a, "r");
    FILE* new = fopen(b, "w");

    char c;
    while ((c = fgetc(old)) != EOF){
        fputc(c, new);
    }
    fclose(old);
    fclose(new);
}

void recursive_duplicateDirectory(char* dirpath_a, char* dirpath_b){
    DIR* dir_a = opendir(dirpath_a);
    DIR* dir_b = opendir(dirpath_b);
    struct dirent * handle;

    if (dir_a == NULL || dir_b == NULL){
        printf("Error opening directory with path name: [%s] in recursive Behavior\n", dir_a);
        return;
    }

    while ((handle = readdir(dir_a)) != NULL){
        if (!in_ignore_list(handle->d_name) || strcmp(handle->d_name, ".history") == 0){
            if (handle->d_type == DT_DIR){
                append_file_path(dirpath_a, handle->d_name);
                append_file_path(dirpath_b, handle->d_name);
                mkdir(dirpath_b, 0777);
                recursive_duplicateDirectory(dirpath_a, dirpath_b);
                delete_last_file_path(dirpath_a, handle->d_name);
                delete_last_file_path(dirpath_b, handle->d_name);
            } else if (handle->d_type == DT_REG){
                append_file_path(dirpath_a, handle->d_name);
                append_file_path(dirpath_b, handle->d_name);
                printf("Duplicating files [%s]\n", dirpath_a);
                duplicate(dirpath_a, dirpath_b);
                delete_last_file_path(dirpath_a, handle->d_name);
                delete_last_file_path(dirpath_b, handle->d_name);
            }
        }
    }
    closedir(dir_a);
    closedir(dir_b);
}

void update_manifest_version(char* project, char* newversion){
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, append); //NEW
    //strcpy(buffer, "."); //OLD
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");

    long length = get_file_length(buffer);
    //char* oldstream = malloc(sizeof(char) * (length+1));
    //bzero(oldstream, sizeof(char) * (length+1));

    FILE* manifestFD = fopen(buffer, "r");
    if (manifestFD == NULL){
        printf("error in opening manifest\n");
        return;
    }

    //fread(oldstream, 1, length, manifestFD);

    
    //char* token = strtok(oldstream, "\n");
    char* newstream = malloc(sizeof(char) * (length+1));
    bzero(newstream, sizeof(char) * (length+1));

    
    strcat(newstream, newversion); //Writes the new version of the manifest
    char temp[PATH_MAX];
    bzero(temp, sizeof(temp));
    fgets(temp, ++length, manifestFD); //Holds the old version of the manifest, newline terminated
    bzero(temp, sizeof(temp));

    while (fgets(temp, length, manifestFD) != NULL){
        strcat(newstream, temp);
        bzero(temp, sizeof(temp));
    }
    /*
    strcat(newstream, "\n");

    while((token = strtok(NULL, "\n")) != NULL){
        strcat(newstream, token);
        strcat(newstream, "\n");
    }
    */
    fclose(manifestFD);
    FILE* newManifestFD = fopen(buffer, "w");
    if (newManifestFD == NULL){
        printf("error in opening new manifest\n");
        return;
    }
    int newSize = strlen(newstream);
    fwrite(newstream, 1, newSize, newManifestFD);
    free(newstream);
    fclose(newManifestFD);
}

void delete_line_from_manifest(char* project, char* file, char* hash){
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, append); //NEW
    //strcpy(buffer, "."); //OLD
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
    fgets(buffer, ++length, manifestFD); //Throwing away first link
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
    
    fclose(manifestFD);
    FILE* newManifestFD = fopen(buffercopy, "w");
    if (newManifestFD == NULL){
        printf("error in opening new manifest\n");
        return;
    }
    int newSize = strlen(newstream);
    fwrite(newstream, 1, newSize, newManifestFD);
    fclose(newManifestFD);
}

void increment_version_number(int version, char* project, char* file, char* hash){
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, append); //NEW
    //strcpy(buffer, "."); //OLD
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    long length = get_file_length(buffer);
    //char* oldstream = malloc(sizeof(char) * (length+1));
    //bzero(oldstream, sizeof(char) * (length+1));
    FILE* manifestFD = fopen(buffer, "r");
    if (manifestFD == NULL){
        printf("error in opening manifest\n");
        return;
    }
    //fread(oldstream, 1, length, manifestFD);

    //char* token = strtok(oldstream, "\n");
    char* newstream = malloc(sizeof(char) * (length+1));
    bzero(newstream, sizeof(char) * (length+1));
    char temp[PATH_MAX];
    bzero(temp, sizeof(temp));
    fgets(temp, ++length, manifestFD);
    strcat(newstream, temp);
    bzero(temp, sizeof(temp));
    //strcat(newstream, token);
    //strcat(newstream, "\n");
    while(fgets(temp, ++length, manifestFD) != NULL){
        char copy[PATH_MAX];
        strcpy(copy, temp);
        bool p = false, f = false;
        char* delim = " \n";
        char* token = strtok(temp, delim); //Holds the file version number
        char version_s[NAME_MAX];
        strcpy(version_s, token);

        token = strtok(NULL, delim); //Holds the project name
        if (strcmp(token, project) == 0) p = true;
        token = strtok(NULL, delim); //Holds the filename
        if (strcmp(token, file) == 0) f = true;
        
        if (p && f){
            //Increment version
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
            strcat(newstream, copy);
        }
    }

    /*
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
    */
    fclose(manifestFD);
    FILE* newManifestFD = fopen(buffer, "w");
    if (newManifestFD == NULL){
        printf("error in opening new manifest\n");
        return;
    }
    int newSize = strlen(newstream);
    fwrite(newstream, 1, newSize, newManifestFD);
    fclose(newManifestFD);
    free(newstream);
}

void testadd(char* project, char* filename){
    //Error checking
    if (!project_exists_on_server(project)){
        printf("Project does not exist on server\n");
        return;
    }
    //Check if file and Manifest exists
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, append); //NEW
    //strcpy(buffer, ".");  //OLD
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    if (!file_exists(filename)){
        char temp[PATH_MAX];
        strcpy(temp, append); //NEW
        //strcpy(temp, ".");  //OLD
        append_file_path(temp, project);
        append_file_path(temp, filename);
        strcpy(filename, temp);
    } else {
        //The filename could be either ./project1/blah.txt or project1/blah.txt...If its the later we want to add ./
        if (filename[0] != '.'){
            char temp[PATH_MAX];
            bzero(temp, sizeof(temp));
            strcat(temp, "./");
            strcat(temp, filename);
            strcpy(filename, temp);
        }
    }

    if (file_exists(filename) && file_exists(buffer)){
        //Check if file is already in the manifest, if so delete it and we will replace it with the updated file
        if (included_in_manifest(project, filename)){
            delete_line_from_manifest(project, filename, NULL);
        }
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
    } else {
        printf("Either the .manifest or the file does not exist\n");
    }
}

void replace_str(char* str, char* replacefrom, char* replaceto){
    /*
    printf("IM in replace str\n");
    printf("str = [%s]\n", str);
    printf("replacefrom = [%s]\n", replacefrom);
    printf("replaceto = [%s]\n", replaceto);
    */
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    int length = strlen(str);
    char* p = strstr(str, replacefrom);
    int length_s = strlen(p);
    int target = length - length_s;
    int length_replace = strlen(replacefrom);
    strncpy(buffer, str, target); 
    strcat(buffer, replaceto);
    strcat(buffer, str+target+length_replace);
    strcpy(str, buffer);
}


void applyCommit(char* commit, char* project, int socket){

    //Create a copy directory with the old manifest version appened to its name
    char buffer[PATH_MAX];
    strcpy(buffer, append); //NEW
    //strcpy(buffer, ".");  //OLD
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    FILE* manifest = fopen(buffer, "r");
    long length_of_old_manifest = get_file_length(buffer);
    bzero(buffer, sizeof(buffer));
    //Insert first line of server manifest into buffer
    fgets(buffer, ++length_of_old_manifest, manifest);
    char* delim = "\n";
    char* token = strtok(buffer, delim);
    //token now holds the old manifest version
    char old_project[PATH_MAX];
    strcpy(old_project, append); //NEW
    //strcpy(old_project, ".");  //OLD
    append_file_path(old_project, project);
    strcat(old_project, "_");
    strcat(old_project, token);
    //old _project is the name of the project with the old version number attached to it.
    mkdir(old_project, 0777);
    rewind(manifest);
    fclose(manifest);
    //Now we have project and empty project_<oldversion>, now we need to recursively through project and copy every file into project_<oldversion>
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, append); //NEW
    //strcpy(buffer, ".");  //OLD
    append_file_path(buffer, project);
    recursive_duplicateDirectory(buffer, old_project);
    //Now that the project is duplicated, apply the changes to the project
    bzero(buffer, sizeof(buffer));
    long length_of_commit = get_file_length(commit);
    FILE* commitfd = fopen(commit, "r");
    char historypath[PATH_MAX];
    strcpy(historypath, append); //NEW
    //strcpy(historypath, ".");  //OLD
    append_file_path(historypath, project);
    append_file_path(historypath, ".history");
    FILE* history = fopen(historypath, "a"); // .history file to append each line to
    bzero(historypath, sizeof(historypath));
    strcpy(historypath, "Manifest version: ");
    fgets(buffer, ++length_of_commit, commitfd); //buffer contains the first line of the .Commit which is the version number and newline
    strcat(historypath, buffer);
    fputs(historypath, history);
    //char* delim_special = "\n";
    //char* token_special = strtok(buffer, delim_special);
    update_manifest_version(project, buffer);
    bzero(buffer, sizeof(buffer));
    //The manifest now has the same version as the .Commit file indicated
    while (fgets(buffer, length_of_commit, commitfd) != NULL){
        char copy[PATH_MAX];
        strcpy(copy, buffer);
        char* delim = " \n";
        char* token = strtok(buffer, delim);
        //Adds M, A, or D
        char mode = token[0];
        //Adds the filepath
        token = strtok(NULL, delim);
        char filepath[PATH_MAX];
        strcpy(filepath, token);
        //create the local filepath
        char localfilepath[PATH_MAX]; //ONLY because of testing purpose
        strcpy(localfilepath, filepath);
        printf("Before changing localfilepath [%s]\n", localfilepath);
        if (strstr(localfilepath, "client1") != NULL){
            replace_str(localfilepath, "client1", "server");
        } else if(strstr(localfilepath, "client2") != NULL){
            replace_str(localfilepath, "client2", "server");
        }
        printf("Client filepath [%s], Local filepath [%s]\n", filepath, localfilepath);
        //Adds the hash
        token = strtok(NULL, delim);
        char hash[PATH_MAX];
        strcpy(hash, token);
        //Adds the new version
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
                create_Directory_Path(localfilepath);
                bzero(sendrequest, sizeof(buffer));
                n = 0;
                while (read(socket, &c, 1) != 0 && c != ':'){
                    sendrequest[n++] = c;
                }
                long file_size = strtol(sendrequest, NULL, 0);
                FILE* add = fopen(localfilepath, "w");
                if (add == NULL){
                    printf("The server does not have file %s in project %s\n", localfilepath, project);
                    bzero(buffer, sizeof(buffer));
                    continue;
                }
                while (file_size > 0 && read(socket, &c, 1) != 0){
                    fwrite(&c, 1, 1, add);
                    file_size--;
                }
                fclose(add);
            } else if (strcmp(sendrequest, "fail") == 0){
                //The client does not have this file
                printf("The client could not find file %s in project %s\n", localfilepath, project);
                bzero(buffer, sizeof(buffer));
                continue;
            }
            if (mode == 'A'){
                testadd(project, localfilepath);
            }
            int version = atoi(version_s);
            increment_version_number(version, project, localfilepath, hash);
        } else if (mode == 'D'){
            delete_line_from_manifest(project, localfilepath, hash);
        }
        fputs(copy, history);
        bzero(buffer, sizeof(buffer));
    }
    fputc('\n', history);
    fclose(history);
    fclose(commitfd);
}

void push(char* project, int socket){
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    //Error checking to see if project exists on server
    if (!project_exists_on_server(project)){
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }
    int n = 0;
    char c;
    bzero(path, sizeof(path));
    char host[PATH_MAX];
    while (true){
        while (read(socket, &c, 1) != 0 && c != ':'){
            path[n++] = c;
        }
        
        if (strcmp(path, "requestfile") == 0){
            bzero(path, sizeof(path));
            n = 0;
            while (read(socket, &c, 1) != 0 && c != ':'){
                path[n++] = c;
            }
            write_bytes_to_socket(path, socket, true);
        } else if (strcmp(path, "done") == 0){
            break;
        } else if (strcmp(path, "host") == 0){
            bzero(path, sizeof(path));
            n = 0;
            while (read(socket, &c, 1) != 0 && c != ':'){
                path[n++] = c;
            }
            strcpy(host, append);
            append_file_path(host, project);
            strcat(host, "_");
            strcat(host, path);
            //Check if both .Commits are the same
            char tempCommit[PATH_MAX];
            strcpy(tempCommit, append);
            append_file_path(tempCommit, ".tempCommit");
            FILE* clientcommit = fopen(tempCommit, "w+");
            bzero(path, sizeof(path));
            n = 0;
            while (read(socket, &c, 1) != 0 && c != ':'){
                path[n++] = c;
            }
            if (strcmp(path, "sendfile") == 0){
                bzero(path, sizeof(path));
                n = 0;
                while (read(socket, &c, 1) != 0 && c != ':'){
                    path[n++] = c;
                }
                long length = strtol(path, NULL, 0);
                while (length > 0 && read(socket, &c, 1) != 0){
                    fwrite(&c, 1, 1, clientcommit);
                    length--;
                }
                //Check if server has commit for host
                if (!file_exists(host)){
                    write(socket, "fail:", 5);
                    fclose(clientcommit);
                    return;
                }
                //Client commit is now the same as the .Commit on the client, check to see if its the same as the server commit.
                FILE* localCommit = fopen(host, "r");
                if (file_same(clientcommit, localCommit)){
                    //Expire all other commits...meaning delete all other
                    fclose(localCommit);
                    expire_all_other_commits(host, project);
                    applyCommit(host, project, socket);
                    remove(host);
                } else {
                    //active commit and commit send by client are not the same
                    printf("The commit given by the client and lost active commit are not the same\n");
                    fclose(localCommit);
                    fclose(clientcommit);
                    remove(tempCommit);
                    write(socket, "fail:", 5);
                    return;
                }
                write(socket, "success:", 8);
            }
            fclose(clientcommit);
            remove(tempCommit);
            return;
        } else if (strcmp(path, "fail") == 0){
            printf("Client failed to find it's commit file\n");
            return;
        }
        bzero(path, sizeof(path));
        n = 0;
    }
}

void destroy(char* project, int socket){
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    //Error checking to see if project exists on server
    if (!project_exists_on_server(project)){
        printf("Project %s does not exist on the server\n", project);
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }
    //Get host information from client
    char host[NAME_MAX];
    int n = 0;
    char c;
    bzero(path, sizeof(path));
    while (read(socket, &c, 1) != 0 && c != ':'){
        path[n++] = c;
    }
    if (strcmp(path, "host") == 0){
        bzero(path, sizeof(path));
        n = 0;
        while (read(socket, &c, 1) != 0 && c != ':'){
            path[n++] = c;
        }
    }
    strcpy(host, project);
    strcat(host, "_");
    strcat(host, path);
    bzero(path, sizeof(path));
    //expire all pending commits
    expire_all_other_commits(host, project);
    //Destory the directory
    strcpy(path, append); //NEW
    //strcpy(path, "."); //OLD
    append_file_path(path, project);
    if (recursively_destory_directory(path) == -1){
        write(socket, "fail:", 5);
        return;
    }
    if (rmdir(path) == -1){
        write(socket, "fail:", 5);
        return;
    }
    write(socket, "success:", 8);
}

void currentversion(char* project, int socket){
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    //Error checking to see if project exists on server
    if (!project_exists_on_server(project)){
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }

    //Open the manifest and feed into client
    bzero(path, sizeof(path));
    strcpy(path, append); //NEW
    //strcpy(path, ".");  //OLD
    append_file_path(path, project);
    append_file_path(path, ".Manifest");
    long length = get_file_length(path);
    FILE* mani = fopen(path, "r");
    if (mani == NULL){
        //No manifest
        printf("No manfiest in project %s", project);
        write(socket, "fail:", 5);
        return;
    }
    bzero(path, sizeof(path));
    strcpy(path, "List of files under project ");
    strcat(path, project);
    strcat(path, "\n:");
    write(socket, path, strlen(path));
    bzero(path, sizeof(path));
    fgets(path, ++length, mani); // To throw away the first line in the manifest
    bzero(path, sizeof(path));
    while (fgets(path, length, mani) != NULL){
        char buffer[PATH_MAX];
        strcpy(buffer, "File Version = [");
        char* delim = " ";
        char* token = strtok(path, delim); //Holds the file version
        strcat(buffer, token);
        strcat(buffer, "], File path = [");
        token = strtok(NULL, delim); //Throw away the project info
        token = strtok(NULL, delim); //Holds the filepath
        strcat(buffer, token);
        strcat(buffer, "]\n:");
        write(socket, buffer, strlen(buffer));
    }
    write(socket, "done:", 5);
    fclose(mani);
}

void delete_higher_rollbacks(char* project, char* version){
    int version_int = atoi(version);
    DIR* dir = opendir(append); //NEW
    //DIR* dir = opendir(".");  //OLD
    if (dir == NULL){
        printf("error opening current direcotry\n");
        return;
    }
    struct dirent * handle;

    while ((handle = readdir(dir)) != NULL){
        if (!in_ignore_list(handle->d_name) && handle->d_type == DT_DIR){
            char copy[PATH_MAX];
            strcpy(copy, handle->d_name);
            char* delim = "_";
            char* project_name = strtok(copy, delim);
            char* version_name = strtok(NULL, delim);
            if (project_name != NULL && version_name != NULL){
                int version_name_int = atoi(version_name);
                if (strcmp(project_name, project) == 0 && version_name_int > version_int){
                    //We found a rollback that is a higher version, delete this
                    char buffer[PATH_MAX];
                    strcpy(buffer, append); //NEW
                    //strcpy(buffer, ".");  //OLD
                    append_file_path(buffer, handle->d_name);
                    recursively_destory_directory(buffer);
                    rmdir(buffer);        
                }
            }
        }
    }
}

bool perform_rollback(char* project, char* version){
    DIR* dir = opendir(append); //NEW
    //DIR* dir = opendir(".");  //OLD
    if (dir == NULL){
        return false;
    }
    struct dirent * handle;

    while ((handle = readdir(dir)) != NULL){
        if (!in_ignore_list(handle->d_name) && handle->d_type == DT_DIR){
            char copy[PATH_MAX];
            strcpy(copy, handle->d_name);
            char* delim = "_";
            char* project_name = strtok(copy, delim);
            char* version_name = strtok(NULL, delim);
            if (project_name != NULL && version_name != NULL){
                if (strcmp(project_name, project) == 0 && strcmp(version_name, version) == 0){
                    //We found our rollback, delete all other rollbacks that are higher than this and rename this to just the project name
                    delete_higher_rollbacks(project, version);
                    char buffer[PATH_MAX];
                    strcpy(buffer, append); //NEW
                    //strcpy(buffer, "."); //OLD
                    append_file_path(buffer, project);
                    recursively_destory_directory(buffer); //Empty original project;
                    rmdir(buffer); //Delete origin project;
                    delete_last_file_path(buffer, project);
                    append_file_path(buffer, handle->d_name);
                    char renameto[PATH_MAX];
                    bzero(renameto, sizeof(rename));
                    strcpy(renameto, append);
                    append_file_path(renameto, project);
                    rename(buffer, renameto);
                    return true;
                }
            }
        }
    }
    return false;
}


void rollback(char* project, int socket, char* version){
    //Error check to see if project exists on server
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    //Error checking to see if project exists on server
    if (!project_exists_on_server(project)){
        printf("%s does not exist on the server\n", project);
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }
    //printf("Version %s\n", version);
    if (!perform_rollback(project, version)){
        printf("Server failed to rollback %s\n", project);
        write(socket, "fail:", 5);
        return;
    }
    write(socket, "done:", 5);
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
            fclose(fp);
            return true;
        } else {
            fclose(fp);
            return false;
        }
    }
}

void history(char* project, int socket){
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    //Error checking to see if project exists on server
    if (!project_exists_on_server(project)){
        printf("%s does not exist on server\n", project);
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }

    //Open the history and feed into client
    bzero(path, sizeof(path));
    strcpy(path, append); //NEW
    //strcpy(path, "."); //OLD
    append_file_path(path, project);
    append_file_path(path, ".history");
    if (file_exists(path) && is_file_empty(path)){
        printf(".history for file %s is empty\n", project);
        write(socket, "empty:", 6);
        return;
    }
    long length = get_file_length(path);
    FILE* history = fopen(path, "r");
    if (history == NULL){
        //No history file
        printf("No .history in %s\n", project);
        write(socket, "fail:", 5);
        return;
    }
    bzero(path, sizeof(path));
    strcpy(path, project);
    strcat(path, "'s History\n:");
    write(socket, path, strlen(path));
    bzero(path, sizeof(path));

    while (fgets(path, length, history) != NULL){
        char copy[PATH_MAX];
        strcpy(copy, path);
        char* token = strtok(copy, " ");
        if (token != NULL && strlen(token) == 1 && (token[0] == 'A' || token[0] == 'D' || token[0] == 'M')){
            char buffer[PATH_MAX];

            strcpy(buffer, token);
            strcat(buffer, " ");
            token = strtok(NULL, " "); //Contains the <file path>
            strcat(buffer, token);
            strcat(buffer, "\n:");
            write(socket, buffer, strlen(buffer));
        } else {
            strcat(path, ":");
            write(socket, path, strlen(path));
        }
        bzero(path, sizeof(path));
    }
    write(socket, "done:", 5);
    fclose(history);
}

pthread_mutex_t select_mutex(char* project){
    printf("I am trying to select a mutex\n");
    pthread_mutex_lock(&list_lock);
    mutex_node * ptr = head;
    //We need to find the node in the mutex list with the same project name
    while (ptr != NULL){
        if (strcmp(ptr->project, project) == 0){
            //We found our correct mutex
            printf("I have succesfully selected a mutex\n");
            pthread_mutex_unlock(&list_lock);
            return ptr->mutex;
        }
        ptr = ptr->next;
    }
    //If we got to this point there isn't a node in the mutex list with the same project name, so add it
    mutex_node * temp = malloc(sizeof(mutex_node));
    temp->project = malloc(sizeof(char) * NAME_MAX);
    strcpy(temp->project, project);
    pthread_mutex_init(&(temp->mutex), NULL);
    temp->next = head;
    head = temp;
    pthread_mutex_unlock(&list_lock);
    printf("I have succesfully selected a mutex\n");
    return temp->mutex;
}

void* handle_connection(void* socketptr){
    //pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    //pthread_mutex_lock(&mutex);
    int socket = *((int*)socketptr);
    char buffer[NAME_MAX];
    char project_name[NAME_MAX];
    char arg[NAME_MAX];
    bzero(buffer, sizeof(buffer));
    bzero(project_name, sizeof(project_name));
    bzero(arg, sizeof(arg));

    while(true){
        if (read(socket, buffer, sizeof(buffer)) == 0) break;
        printf("The command is [%s]\n", buffer);
        if (strcmp(buffer, "done") == 0){
            printf("Ending connection with client\n");
            break;
        }
        read(socket, project_name, sizeof(project_name));
        pthread_mutex_t mutex = select_mutex(project_name);
        pthread_mutex_lock(&mutex);
        printf("Locked mutex for project %s for socket %d\n", project_name, socket);
        if (strcmp(buffer, "checkout") == 0) {
            checkout(project_name, socket);
        } else if (strcmp(buffer, "update") == 0){
            update(project_name, socket);
        } else if (strcmp(buffer, "upgrade") == 0){
            upgrade(project_name, socket);
        } else if (strcmp(buffer, "commit") == 0){
            commit(project_name, socket);
        } else if (strcmp(buffer, "push") == 0){
            //Lock the repo by locking mutex list_lock
            pthread_mutex_lock(&list_lock); 
            push(project_name, socket);
            pthread_mutex_unlock(&list_lock);
        } else if (strcmp(buffer, "create") == 0){
            create(project_name, socket);
        } else if (strcmp(buffer, "destroy") == 0){
            pthread_mutex_lock(&list_lock);
            destroy(project_name, socket);
            pthread_mutex_unlock(&list_lock);
        } else if (strcmp(buffer, "currentversion") == 0){
            currentversion(project_name, socket);
        } else if (strcmp(buffer, "history") == 0){
            history(project_name, socket);
        } else if (strcmp(buffer, "rollback") == 0){
            read(socket, arg, sizeof(arg));
            rollback(project_name, socket, arg);
        } else {
            printf("couldn't select a command\n");
        }
        pthread_mutex_unlock(&mutex);
        printf("Unlocked mutex for project %s for socket %d\n", project_name, socket); 
        bzero(buffer, sizeof(buffer));
        bzero(project_name, sizeof(project_name));
        bzero(arg, sizeof(arg));
    }

    close(socket);
    //pthread_mutex_unlock(&mutex);
    return NULL;
}


int main(int argc, char** argv){
    //intialize append
    char buffer[PATH_MAX];
    strcpy(buffer, argv[0]);
    delete_last_file_path(buffer, "WTFserver");
    append = malloc(sizeof(char) * (strlen(buffer)+1));
    strcpy(append, buffer);
    printf("I will append [%s] to every .\n", append);
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
        
        pthread_t newthread;
        int* pclient = malloc(sizeof(int));
        *pclient = clientSocket; 
        pthread_create(&newthread, NULL, handle_connection, pclient);
        //handle_connection(clientSocket);
    }



    
    chatFunction(clientSocket);
    close(socketFD);
    free(append);
    return 0;
}

