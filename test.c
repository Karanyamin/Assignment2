#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void replace_str(char* str, char* replacefrom, char* replaceto){
    char buffer[PATH_MAX];
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

int main(){
    char filepath[PATH_MAX];
    strcpy(filepath, "./client1/project1/america.txt");

    char localfilepath[PATH_MAX]; //ONLY because of testing purpose
        strcpy(localfilepath, filepath);
        if (strstr(localfilepath, "client1") != NULL){
            replace_str(localfilepath, "client1", "server");
        } else if(strstr(localfilepath, "client2") != NULL){
            replace_str(localfilepath, "client2", "server");
        }
        printf("Client filepath [%s], Local filepath [%s]\n", filepath, localfilepath);
    /*
    printf("[%s]\n", buffer);
    
    replace_str(buffer, "client1", "server");
    printf("[%s]\n", buffer);
    
    if (strstr(buffer, "client1") != NULL){
        replace_str(buffer, "client1", "server");
    }
    printf("[%s]\n", buffer);
    */
    return 0;
}