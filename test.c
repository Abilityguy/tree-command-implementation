#include<stdio.h>
#include<string.h>
#include<stdlib.h>

const char *concat(char* s1, char* s2){
    char *ns = malloc(strlen(s1) + strlen(s2) + 1);
    ns[0] = '\0';
    strcat(ns, s1);
    strcat(ns, s2);
    return ns;
}


int main(){
    char* path = "abcdasd";
    printf("%s", concat(path, concat("/", "achar")));
}