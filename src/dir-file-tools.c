#include "dir-file-tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>


void validateDirectoryPath(const char *directory_path){
    DIR *directory = opendir(directory_path);
    if (directory == NULL){
        if (errno == ENOENT){
            printf(ERR_NO_SUCH_DIR, directory_path);
        }
        else if (errno == EACCES){
            printf(ERR_ACCES, directory_path);
        }
        else if (errno == ENOTDIR){
            printf(ERR_NOT_DIR, directory_path);
        }
        else{
            perror("Ошибка");
        }
        exit(EXIT_FAILURE);
    }
    else{
        closedir(directory);
    }
}

void validateFilePath(const char *integrity_list_path){
    FILE *integrity_list = fopen(integrity_list_path, "r+");
    if (integrity_list == NULL){
        if (errno == ENOENT){
            printf(ERR_NO_INT_LIST, integrity_list_path);
        }
        else if (errno == EACCES){
            printf(ERR_ACCES, integrity_list_path);
        }
        else if (errno == EISDIR){
            printf(ERR_NOT_FILE, integrity_list_path);
        }
        else{
            perror("Ошибка");
        }
        exit(EXIT_FAILURE);
    }
    else{
        fclose(integrity_list);
    }    
}

bool isDirectory(const char *path){
    DIR *dir = opendir(path);
    if (dir){
        closedir(dir);
        return true;
    }
    else{
        return false;
    }
}