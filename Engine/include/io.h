#ifndef QE_IO_H
#define QE_IO_H

typedef struct {
    char** filenames;
    int count;
} QIO_DirList;

int qExists(char* path);
int qIsDir(char* path);
int qCreateDir(char* path);
int qCreateDirRecursive(char* path);
char* qReadTextFile(char* filename);
int qWriteTextFile(char* filename, char* content);
QIO_DirList qListDir(char* dir_path);
void qFreeDirList(QIO_DirList* list);

#endif
