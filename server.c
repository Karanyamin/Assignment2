
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
char* int_to_string(long num, bool delim);
bool is_IPv4_address(const char* s);
void append_file_path(char* path, char* nextDirectoryName);
void delete_last_file_path(char* path, char* nextDirectoryName);

bool included_in_manifest(char* project, char* file){
    char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
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
    fgets(buffer, length, manifestFD); //Throwing away first link
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
                if (remove(path) != 0) return -1;
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
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    long length = get_file_length(buffer);
    FILE* manifestFD = fopen(buffer, "r");
    if (manifestFD == NULL){
        printf("error in opening manifest\n");
        return 0;
    }
   
    bzero(buffer, sizeof(buffer));

    while (fgets(buffer, length, manifestFD) != NULL){
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
    strcpy(path, "./");
    strcat(path, project);
    if(write_all_files_to_socket_in_sequence(path, socket, project) == 0){
        printf("Error in writing all filepath names to socket\n");
        return;
    }
    //write all bytes of all files in order
    bzero(path, sizeof(path));
    strcpy(path, "./");
    strcat(path, project);
    write_bytes_of_all_files(path, socket, project);
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
    //Creates the .Manifest file and puts it in the project
    int fd = open(path, O_WRONLY | O_CREAT, 00600);
    if (fd == -1){
        printf("The server failed to create the Manifest in the directory\n");
        return;
    }
    delete_last_file_path(path, ".Manifest");
    append_file_path(path, ".history");
    int historyfd = open(path, O_WRONLY | O_CREAT, 00600);
    if (historyfd == -1){
        printf("The server failed to create the .history file in project %s\n", project_name);
        return;
    }
    write(fd, "1\n", 2);
    close(fd);
    close(historyfd);
}

void update(char* project, int socket){
    //Error checking to see if project exists on server
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
    write(socket, "sendfile:", 9);
    bzero(path, sizeof(path));
    strcpy(path, ".");
    append_file_path(path, project);
    append_file_path(path, ".Manifest");
    char* length_of_manifest = int_to_string(get_file_length(path), true);
    write(socket, length_of_manifest, strlen(length_of_manifest));
    if (write_bytes_to_socket(path, socket, false) == 0){
        printf("Error in sending manifest to client\n");
        return;
    }
    write(socket, "done:", 6);
}

void commit(char* project, int socket){
    char path[PATH_MAX];
    bzero(path, sizeof(path));
    strcpy(path, ".");
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
    write(socket, "sendfile:", 9);
    bzero(path, sizeof(path));
    strcpy(path, ".");
    append_file_path(path, project);
    append_file_path(path, ".Manifest");
    char* length_of_manifest = int_to_string(get_file_length(path), true);
    write(socket, length_of_manifest, strlen(length_of_manifest));
    if (write_bytes_to_socket(path, socket, false) == 0){
        printf("Error in sending manifest to client\n");
        return;
    }
    int n = 0;
    char c;
    bzero(path, sizeof(path));
    char host[PATH_MAX];
    bzero(host, sizeof(host));
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
            strcpy(host, project);
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
            
        }
        bzero(path, sizeof(path));
        n = 0;
    }

}

bool file_same(FILE* a, FILE* b){
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
    DIR* dir = opendir(".");
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
        exit(1);
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

void testadd(char* project, char* filename){
    //Error checking
    if (!project_exists_on_server(project)){
        printf("Project does not exist on server\n");
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


void applyCommit(char* commit, char* project, int socket){

    //Create a copy directory with the old manifest version appened to its name
    char buffer[PATH_MAX];
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    append_file_path(buffer, ".Manifest");
    FILE* manifest = fopen(buffer, "r");
    long length_of_old_manifest = get_file_length(buffer);
    bzero(buffer, sizeof(buffer));
    //Insert first line of server manifest into buffer
    fgets(buffer, length_of_old_manifest, manifest);
    char* delim = "\n";
    char* token = strtok(buffer, delim);
    //token now holds the old manifest version
    char old_project[PATH_MAX];
    strcpy(old_project, ".");
    append_file_path(old_project, project);
    strcat(old_project, "_");
    strcat(old_project, token);
    //old _project is the name of the project with the old version number attached to it.
    mkdir(old_project, 0777);
    rewind(manifest);
    //Now we have project and empty project_<oldversion>, now we need to recursively through project and copy every file into project_<oldversion>
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, ".");
    append_file_path(buffer, project);
    recursive_duplicateDirectory(buffer, old_project);
    //Now that the project is duplicated, apply the changes to the project
    bzero(buffer, sizeof(buffer));
    long length_of_commit = get_file_length(commit);
    FILE* commitfd = fopen(commit, "r");
    char historypath[PATH_MAX];
    strcpy(historypath, ".");
    append_file_path(historypath, project);
    append_file_path(historypath, ".history");
    FILE* history = fopen(historypath, "a"); // .history file to append each line to
    bzero(historypath, sizeof(historypath));
    strcpy(historypath, "Manifest version: ");
    fgets(buffer, length_of_commit, commitfd); //buffer contains the first line of the .Commit which is the version number and newline
    strcat(historypath, buffer);
    fputs(historypath, history);
    char* delim_special = "\n";
    char* token_special = strtok(buffer, delim_special);
    update_manifest_version(project, token_special);
    bzero(buffer, sizeof(buffer));
    //The manifest now has the same version as the .Commit file indicated
    while (fgets(buffer, length_of_commit, commitfd) != NULL){
        fputs(buffer, history);
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
                    fclose(add);
                    fclose(commitfd);
                    fclose(history);
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
            strcpy(host, project);
            strcat(host, "_");
            strcat(host, path);
            //Check if both .Commits are the same
        
            FILE* clientcommit = fopen(".tempCommit", "w");
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
                    return;
                }
                //Client commit is now the same as the .Commit on the client, check to see if its the same as the server commit.
                FILE* localCommit = fopen(host, "r");
                if (!file_same(clientcommit, localCommit)){
                    //Expire all other commits...meaning delete all other
                    fclose(localCommit);
                    expire_all_other_commits(host, project);
                    applyCommit(host, project, socket);
                    remove(host);
                } else {
                    fclose(localCommit);
                }
                write(socket, "success:", 8);
            }
            fclose(clientcommit);
            remove(".tempCommit");
            break;
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
    strcpy(path, ".");
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
    strcpy(path, ".");
    append_file_path(path, project);
    append_file_path(path, ".Manifest");
    long length = get_file_length(path);
    FILE* mani = fopen(path, "r");
    if (mani == NULL){
        //No manifest
        write(socket, "fail:", 5);
        return;
    }
    bzero(path, sizeof(path));
    strcpy(path, "List of files under project ");
    strcat(path, project);
    strcat(path, "\n:");
    write(socket, path, strlen(path));
    bzero(path, sizeof(path));
    fgets(path, length, mani); // To throw away the first line in the manifest
    bzero(path, sizeof(path));
    while (fgets(path, length, mani) != NULL){
        char buffer[PATH_MAX];
        strcpy(buffer, "File Version = [");
        char* delim = " ";
        char* token = strtok(path, delim);
        strcat(buffer, token);
        strcat(buffer, "], File path = [");
        token = strtok(NULL, delim); //Throw away the project info
        token = strtok(NULL, delim);
        strcat(buffer, token);
        strcat(buffer, "]\n:");
        write(socket, buffer, strlen(buffer));
    }
    write(socket, "done:", 5);
}

void delete_higher_rollbacks(char* project, char* version){
    int version_int = atoi(version);
    DIR* dir = opendir(".");
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
                    strcpy(buffer, ".");
                    append_file_path(buffer, handle->d_name);
                    recursively_destory_directory(buffer);
                    rmdir(buffer);        
                }
            }
        }
    }
}

bool perform_rollback(char* project, char* version){
    DIR* dir = opendir(".");
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
                    strcpy(buffer, ".");
                    append_file_path(buffer, project);
                    recursively_destory_directory(buffer); //Empty original project;
                    rmdir(buffer); //Delete origin project;
                    delete_last_file_path(buffer, project);
                    append_file_path(buffer, handle->d_name);
                    rename(buffer, project);
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
        strcpy(path, "fail");
        write(socket, path, sizeof(path));
        return;
    } else {
        strcpy(path, "success");
        write(socket, path, sizeof(path));
    }
    //printf("Version %s\n", version);
    if (!perform_rollback(project, version)){
        write(socket, "fail:", 5);
        return;
    }
    write(socket, "done:", 5);
}

void history(char* project, int socket){
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
    strcpy(path, ".");
    append_file_path(path, project);
    append_file_path(path, ".history");
    long length = get_file_length(path);
    FILE* history = fopen(path, "r");
    if (history == NULL){
        //No history file
        write(socket, "fail:", 5);
        return;
    }
    bzero(path, sizeof(path));
    strcpy(path, project);
    strcat(path, "'s History\n:");
    write(socket, path, strlen(path));
    bzero(path, sizeof(path));

    fgets(path, length, history);
    strcat(path, ":");
    write(socket, path, strlen(path));
    bzero(path, sizeof(path));

    while (fgets(path, length, history) != NULL){
        char buffer[PATH_MAX];
        char* delim = " ";
        char* token = strtok(path, delim); //Contains the operator
        strcpy(buffer, token);
        strcat(buffer, " ");
        token = strtok(NULL, delim); //Contains the <file path>
        strcat(buffer, token);
        strcat(buffer, "\n:");
        write(socket, buffer, strlen(buffer));
    }
    write(socket, "done:", 5);
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
            read(socket, project_name, sizeof(project_name));
            update(project_name, socket);
        } else if (strcmp(buffer, "upgrade") == 0){
            read(socket, project_name, sizeof(project_name));
            upgrade(project_name, socket);
        } else if (strcmp(buffer, "commit") == 0){
            read(socket, project_name, sizeof(project_name));
            commit(project_name, socket);
        } else if (strcmp(buffer, "push") == 0){
            read(socket, project_name, sizeof(project_name));
            push(project_name, socket);
        } else if (strcmp(buffer, "create") == 0){
            read(socket, project_name, sizeof(project_name));
            create(project_name, socket);
        } else if (strcmp(buffer, "destroy") == 0){
            read(socket, project_name, sizeof(project_name));
            destroy(project_name, socket);
        } else if (strcmp(buffer, "currentversion") == 0){
            read(socket, project_name, sizeof(project_name));
            currentversion(project_name, socket);
        } else if (strcmp(buffer, "history") == 0){
            read(socket, project_name, sizeof(project_name));
            history(project_name, socket);
        } else if (strcmp(buffer, "rollback") == 0){
            read(socket, project_name, sizeof(project_name));
            read(socket, arg, sizeof(arg));
            rollback(project_name, socket, arg);
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

