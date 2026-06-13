#include "../include/text.h"
#include "../include/io.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif
int qExists(char* path) { return access(path, F_OK) == 0; }
int qIsDir(char* path) {
    #ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
    #else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
    #endif
}
int qCreateDir(char* path) {
    if (qExists(path)) return qIsDir(path);
    #ifdef _WIN32
    return CreateDirectoryA(path, NULL) != 0;
    #else
    return mkdir(path, 0777) == 0;
    #endif
}
int qCreateDirRecursive(char* path) {
    char temp[512];
    int len = qStrlen(path);
    if (len >= (int)sizeof(temp) - 1) return 0;
    qStrcpy(temp, path);
    for (int i = 0; i <= len; i++) {
        if (temp[i] == '/' || temp[i] == '\\' || temp[i] == '\0') {
            char c = temp[i]; temp[i] = '\0';
            if (qStrlen(temp) > 0) { if (!(qStrlen(temp) == 2 && temp[1] == ':')) { if (!qCreateDir(temp)) return 0; } }
            temp[i] = c;
        }
    }
    return 1;
}

char* qReadTextFile(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length < 0) { fclose(file); return NULL; }
    char* buffer = (char*)malloc(length + 1);
    if (!buffer) { fclose(file); return NULL; }
    size_t read_bytes = fread(buffer, 1, length, file);
    buffer[read_bytes] = '\0';
    fclose(file);
    return buffer;
}

int qWriteTextFile(char* filename, char* content) {
    FILE* file = fopen(filename, "wb");
    if (!file) return 0;

    int length = qStrlen(content);
    size_t written = fwrite(content, 1, length, file);

    fclose(file);
    return (int)written == length;
}

QIO_DirList qListDir(char* dir_path) {
    QIO_DirList list;
    list.filenames = NULL;
    list.count = 0;
    #ifdef _WIN32
    char search_path[512];
    int len = qStrlen(dir_path);
    if (len >= 510) return list;
    qStrcpy(search_path, dir_path);
    if (search_path[len-1] != '/' && search_path[len-1] != '\\') {
        search_path[len] = '/';
        search_path[len+1] = '*';
        search_path[len+2] = '\0';
    } else { search_path[len] = '*'; search_path[len+1] = '\0'; }
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return list;
    int capacity = 0;
    do { if (!qStrcmp(find_data.cFileName, ".") && !qStrcmp(find_data.cFileName, "..")) { capacity++; }
    } while (FindNextFileA(hFind, &find_data));
    if (capacity > 0) {
        list.filenames = (char**)malloc(capacity * sizeof(char*));
        FindClose(hFind);
        hFind = FindFirstFileA(search_path, &find_data);
        do {
            if (!qStrcmp(find_data.cFileName, ".") && !qStrcmp(find_data.cFileName, "..")) {
                int name_len = qStrlen(find_data.cFileName);
                list.filenames[list.count] = (char*)malloc(name_len + 1);
                qStrcpy(list.filenames[list.count], find_data.cFileName);
                list.count++;
            }
        } while (FindNextFileA(hFind, &find_data) && list.count < capacity);
    }
    FindClose(hFind);
    #else
    DIR* d = opendir(dir_path);
    if (!d) return list;
    struct dirent* dir;
    int capacity = 0;
    while ((dir = readdir(d)) != NULL) { if (!qStrcmp(dir->d_name, ".") && !qStrcmp(dir->d_name, "..")) { capacity++; } }
    if (capacity > 0) {
        rewinddir(d);
        list.filenames = (char**)malloc(capacity * sizeof(char*));
        while ((dir = readdir(d)) != NULL && list.count < capacity) {
            if (!qStrcmp(dir->d_name, ".") && !qStrcmp(dir->d_name, "..")) {
                int name_len = qStrlen(dir->d_name);
                list.filenames[list.count] = (char*)malloc(name_len + 1);
                qStrcpy(list.filenames[list.count], dir->d_name);
                list.count++;
            }
        }
    }
    closedir(d);
    #endif
    return list;
}
void qFreeDirList(QIO_DirList* list) {
    if (!list) return;
    if (list->filenames) {
        for (int i = 0; i < list->count; i++) { free(list->filenames[i]); }
        free(list->filenames);
        list->filenames = NULL;
    }
    list->count = 0;
}
