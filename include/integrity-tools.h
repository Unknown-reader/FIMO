#ifndef INTEGRITY_TOOLS_H
#define INTEGRITY_TOOLS_H


#include <stdio.h>
#include <stdbool.h>


#define MAX_FILE_PATH_LEN 4096
#define ALLOWED_CHARS "/ abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-+=*.,!?()[]{}<>~#&%@^|:;"


typedef struct{
    char path[MAX_FILE_PATH_LEN];
    char hash[65];
    bool matched;
} PathHashPair;


void listFilesOnlyRecursively(const char *directory_path, FILE *file);
bool validateIntegrityList(const char *path);
bool checkDirIntegrity(const char *directory_path, const char *integrity_list_path);

#endif