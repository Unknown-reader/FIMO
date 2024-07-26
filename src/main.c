#include "dir-file-tools.h"
#include "integrity-tools.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <syslog.h>


#define SET_USAGE_FMT "\nДля постановки каталога на контроль целостности используйте:\n%s [-s set] 'path/to/dir' 'path/to/control-file'\n"
#define CHECK_USAGE_FMT "\nДля проведения контроля целостности каталога используйте:\n%s [-c check] 'path/to/dir' 'path/to/control-file'\n"

#define ERR_ARG_NUM "Неожиданное количество аргументов! Ожидалось 3, но получено %d.\n"
#define ERR_INVALID_ARG "Неизвестный аргумент '%s'!\n"
#define ERR_BAD_INTEGRITY_LIST "'%s' не является контрольным файлом или целостность контрольного файла нарушена!\n"

#define MSG_INTEGRITY_LIST_CREATED "Контрольный файл для каталога '%s' успешно заполнен!\n"
#define MSG_INTEGRITY_VALIDATED "\nЦелостность каталога '%s' заверена!\n"
#define MSG_INTEGRITY_CORRUPTED "\nОбнаружено нарушение целостности каталога '%s'!\n"


int main(int argc, char* argv[]){
    openlog("FIMO", LOG_PID | LOG_CONS, LOG_USER);

    if (argc != 4){
        printf(ERR_ARG_NUM, argc-1);
        printf(SET_USAGE_FMT, argv[0]);
        printf(CHECK_USAGE_FMT, argv[0]);
        exit(EXIT_FAILURE);
    }
    
    const char *mode = argv[1];
    const char *directory_path = argv[2];
    const char *integrity_list_path = argv[3];
    bool start_control;

    // check that mode-argument is correct
    if ((strlen(mode) == 2 && strncmp(mode, "-s", 2) == 0) || 
        (strlen(mode) == 4 && strncmp(mode, "-set", 4) == 0)){
        start_control = true;
    }
    else if ((strlen(mode) == 2 && strncmp(mode, "-c", 2) == 0) || 
        (strlen(mode) == 6 && strncmp(mode, "-check", 6) == 0)){
        start_control = false;
    }
    else{
        printf(ERR_INVALID_ARG, argv[1]);
        printf(SET_USAGE_FMT, argv[0]);
        printf(CHECK_USAGE_FMT, argv[0]);
        exit(EXIT_FAILURE);
    }

    // check that directory exists
    validateDirectoryPath(directory_path);

    // check that integrity-list exists
    validateFilePath(integrity_list_path);

    // perform set or check integrity command
    if (start_control){
        // create integrity-list
        FILE *integrity_file = fopen(integrity_list_path, "w");
        listFilesOnlyRecursively(directory_path, integrity_file);
        printf(MSG_INTEGRITY_LIST_CREATED, directory_path);
        syslog(LOG_INFO, MSG_INTEGRITY_LIST_CREATED, directory_path);

        fclose(integrity_file);
    }
    else{
        // check directory integrity
        if (!validateIntegrityList(integrity_list_path)){
            printf(ERR_BAD_INTEGRITY_LIST, integrity_list_path);
            exit(EXIT_FAILURE);
        }

        if (checkDirIntegrity(directory_path, integrity_list_path)){
            printf(MSG_INTEGRITY_VALIDATED, directory_path);
            syslog(LOG_INFO, MSG_INTEGRITY_VALIDATED, directory_path);
        }
        else{
            printf(MSG_INTEGRITY_CORRUPTED, directory_path);
            syslog(LOG_INFO, MSG_INTEGRITY_CORRUPTED, directory_path);
        }
    }

    closelog();
    return 0;
}