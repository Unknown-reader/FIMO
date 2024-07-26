#ifndef DIR_FILE_TOOLS_H
#define DIR_FILE_TOOLS_H


#include <stdbool.h>


#define ERR_NO_SUCH_DIR "Не удалось найти каталог '%s'!\n"
#define ERR_NO_INT_LIST "Не удалось найти контрольный файл '%s'!\n"
#define ERR_ACCES "Недостаточно прав для доступа к '%s'!\n"
#define ERR_NOT_DIR "'%s' не является директорией!\n"
#define ERR_NOT_FILE "'%s' является директорией, а не файлом!\n"


void validateDirectoryPath(const char *directory_path);
void validateFilePath(const char *integrity_list_path);

bool isDirectory(const char *path);

#endif