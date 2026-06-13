#ifndef QE_MAIN_H
#define QE_MAIN_H

#define MAX_LINE_LENGTH 4096
#define MORE_LINES 512
typedef struct { char* filename; char** lines; int lineCount; int maxLines; int cursorX; int cursorY; } QCoder;
void openFile(char* filename);
extern QCoder coder;

#endif
