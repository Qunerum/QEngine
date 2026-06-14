#ifndef QE_MAIN_H
#define QE_MAIN_H

#define MAX_FILE_NAME_LEN 1024
#define MAX_FILE_SIZE (1024*1024) // 1024KB , 1MB

extern char coderOpenedFile[MAX_FILE_NAME_LEN];
extern char coderBuffer[MAX_FILE_SIZE];
extern int coderBufferLen, coderCursor;

void qCoderInputChar(char c);
void openFile(char* filename);

#endif
