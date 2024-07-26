#include "integrity-tools.h"
#include "dir-file-tools.h"
#include "sha256.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syslog.h>


#define ERR_FILE_CHANGED "\tФайл '%s' изменен!\n"
#define ERR_FILE_REMOVED "\tФайл '%s' был удален из каталога!\n"
#define ERR_FILE_ADDED "\tФайл '%s' был добавлен в каталог!\n"

#define WRN_FILE_RENAMED "\tФайл '%s' переименован/заменен на переименованную копию или перемещен внутри каталога!\n"


void listFilesOnlyRecursively(const char *directory_path, FILE *file){
    struct dirent *dp;
    DIR *dir = opendir(directory_path);

    while ((dp = readdir(dir)) != NULL){
        char cur_file_path[MAX_FILE_PATH_LEN];
        strcpy(cur_file_path, directory_path);
        strcat(cur_file_path, "/");
        strcat(cur_file_path, dp->d_name);

        if ((strcmp(dp->d_name, ".") != 0) && (strcmp(dp->d_name, "..") != 0)){
            if (!isDirectory(cur_file_path)){
                fprintf(file, "%s\n%s\n", cur_file_path, calculateSHA256(cur_file_path));
            }
            else{
                listFilesOnlyRecursively(cur_file_path, file);
            }
        }
    }
    closedir(dir);
}

bool validateIntegrityList(const char *path){
    FILE *list = fopen(path, "r");

    char line[MAX_FILE_PATH_LEN];
    int line_number = 0;

    // count number of lines
    while (fgets(line, sizeof(line), list)){
        line_number++;
    }

    // check that there is even number of lines
    if ((line_number) % 2 != 0){
        printf("number of lines is not even\n");
        fclose(list);
        return false;
    }

    rewind(list);

    for (int i = 1; i <= line_number-1; ++i){
        fgets(line, sizeof(line), list);

        // remove '\n' from the end of line
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n'){
            line[len-1] = '\0';
        }

        // check that there are no empty lines
        if (strlen(line) == 0){
            fclose(list);
            printf("there are empty lines\n");
            return false;
        }

        if (i % 2 != 0){
            // check path correctness
            if (line[0] != '/'){
                fclose(list);
                printf("path does not start with /\n");
                return false;
            }

            for (size_t i = 0; i < len-1; ++i){
                if (line[i] == '/'){
                    if (i + 1 < len && line[i+1] == '/'){
                        fclose(list);
                        printf("there are two / signs next to\n");
                        return false;
                    }

                    continue;
                }

                if (!strchr(ALLOWED_CHARS, line[i])){
                    fclose(list);
                    printf("invalid char in path\n");
                    return false;
                }
            }
        }
        else{
            // check hash correctness
            if (len-1 != SHA256_HASH_LENGTH * 2){
                fclose(list);
                printf("len of hash not 64\n");
                return false;
            }

            for (size_t i = 0; i < len-1; ++i){
                char c = line[i];
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))){
                    fclose(list);
                    printf("hash contains invalid char\n");
                    return false;
                }
            }
        }
    }

    fclose(list);
    return true;
}

static void readIntegrityList(FILE *file, PathHashPair **arr, int *len){
    
    char path[MAX_FILE_PATH_LEN];
    char hash[65];

    // count number of files
    int line_number = 0;
    while (fscanf(file, "%s %s", path, hash) == 2){
        line_number++;
    }
    rewind(file);

    // allocate memory for arr
    *arr = (PathHashPair *)malloc(line_number * sizeof(PathHashPair));

    // read filepaths and hashes and put into arr
    int ind = 0;
    while (fscanf(file, "%s %s", path, hash) == 2){
        strncpy((*arr)[ind].path, path, MAX_FILE_PATH_LEN);
        strncpy((*arr)[ind].hash, hash, 65);
        (*arr)[ind].matched = false;
        ind++;
    }

    fclose(file);
    *len = line_number;
}

static void printInfoHeader(const char *s, bool flag){
    if (flag){
        printf("-----------------------------\n");
        printf("%s\n", s);
    }
}

bool checkDirIntegrity(const char *directory_path, const char *integrity_list_path){

    bool integrity = true;
    bool warnings = false;

    // save filepaths and hashes from integrity list
    FILE *list = fopen(integrity_list_path, "r");
    PathHashPair *files_in_list;
    int len1;
    readIntegrityList(list, &files_in_list, &len1);

    // get array of current filepaths and hashes in directory
    FILE *temp = tmpfile();
    listFilesOnlyRecursively(directory_path, temp);
    rewind(temp);
    PathHashPair *files_in_dir;
    int len2;
    readIntegrityList(temp, &files_in_dir, &len2);


    // find matches path(+) -- hash(+) and path(+) -- hash(-)
    for (int i = 0; i < len1; ++i){
        for (int j = 0; j < len2; ++j){
            if (!files_in_dir[j].matched){
                // path and hash not changed
                if (strcmp(files_in_dir[j].path, files_in_list[i].path) == 0 &&
                    strcmp(files_in_dir[j].hash, files_in_list[i].hash) == 0){
                    
                    files_in_list[i].matched = true;
                    files_in_dir[j].matched = true;

                    break;
                }
                // path not changed, hash changed
                if (strcmp(files_in_dir[j].path, files_in_list[i].path) == 0 &&
                    strcmp(files_in_dir[j].hash, files_in_list[i].hash) != 0){
                    
                    files_in_list[i].matched = true;
                    files_in_dir[j].matched = true;

                    printInfoHeader("Критические несоответствия:", integrity);
                    printf(ERR_FILE_CHANGED, files_in_list[i].path);
                    syslog(LOG_ERR, ERR_FILE_CHANGED, files_in_list[i].path);
                    integrity = false;
                    break;
                }                
            }
      
        }
    }
    
    PathHashPair *renamed = (PathHashPair *)malloc(len1 * sizeof(PathHashPair));
    int renamed_ind = 0;
    // find renamed files or copies of removed files with different names (tracking directory not corrupted)
    for (int i = 0; i < len1; ++i){
        if (files_in_list[i].matched){
            continue;
        }
        for (int j = 0; j < len2; ++j){
            if (!files_in_dir[j].matched){
                if (strcmp(files_in_dir[j].path, files_in_list[i].path) != 0 &&
                    strcmp(files_in_dir[j].hash, files_in_list[i].hash) == 0){
                    
                    files_in_list[i].matched = true;
                    files_in_dir[j].matched = true;

                    strncpy(renamed[renamed_ind].path, files_in_list[i].path, MAX_FILE_PATH_LEN);
                    strncpy(renamed[renamed_ind].hash, files_in_list[i].hash, 65);
                    renamed_ind++;

                    warnings = true;
                    break;
                }                
            }
        }
    }

    // find deleted files
    for (int i = 0; i < len1; ++i){
        if (!files_in_list[i].matched){
            printInfoHeader("Критические несоответствия:", integrity);
            integrity = false;
            printf(ERR_FILE_REMOVED, files_in_list[i].path);
            syslog(LOG_ERR, ERR_FILE_REMOVED, files_in_list[i].path);
        }
    }

    // find new files
    for (int i = 0; i < len2; ++i){
        if (!files_in_dir[i].matched){
            printInfoHeader("Критические несоответствия:", integrity);
            integrity = false;
            printf(ERR_FILE_ADDED, files_in_dir[i].path);
            syslog(LOG_ERR, ERR_FILE_ADDED, files_in_dir[i].path);
        }
    }

    // print warnings
    if (warnings){
        printInfoHeader("Замечания:", warnings);
        for (int i = 0; i < renamed_ind; ++i){
            printf(WRN_FILE_RENAMED, renamed[i].path);
            syslog(LOG_INFO, WRN_FILE_RENAMED, renamed[i].path);
        }
    }

    free(files_in_list);
    free(files_in_dir);
    free(renamed);

    return integrity;    
}