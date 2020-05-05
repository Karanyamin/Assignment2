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


int main(int argc, char** argv){
    //unsigned char* hash = malloc(sizeof(char) * SHA_DIGEST_LENGTH);
    char buffer[5000000];
    unsigned char hash[SHA_DIGEST_LENGTH];
    int fd = open(argv[1], O_RDONLY);
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
    printf("The hash for the file [%s] is [", argv[1]);
    int i;
    for (i = 0; i < SHA_DIGEST_LENGTH; i++){
        printf("%02x", hash[i]);
    }
    printf("]\n");
    
}