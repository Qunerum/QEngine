#include "../lib/qgpu.h"
#include "../include/main.h"
#include "../include/ui.h"
#include "../include/io.h"
#include "../include/text.h"
#include <stdlib.h>

QCoder coder;
void qCoderOpenFile(char* filename) {
    if (coder.lines) { for (int i=0;i<coder.lineCount;i++) free(coder.lines[i]); free(coder.lines); }
    if (!coder.filename) coder.filename = (char*)malloc(MAX_LINE_LENGTH);
    qStrcpy(coder.filename, filename);
    char* rawText = qReadTextFile(coder.filename);
    if (!rawText) {
        coder.lineCount = 1;
        coder.maxLines = MORE_LINES;
        coder.lines = (char**)malloc(coder.maxLines * sizeof(char*));
        coder.lines[0] = (char*)malloc(MAX_LINE_LENGTH);
        coder.lines[0][0] = '\0';
        return;
    }
    int actualLines = 1;
    for (int i=0;rawText[i];i++) { if (rawText[i] == '\n') actualLines++; }
    coder.lineCount = actualLines;
    coder.maxLines = actualLines + MORE_LINES;
    coder.lines = (char**)malloc(coder.maxLines * sizeof(char*));
    int rawIdx = 0;
    for (int l=0;l<actualLines;l++) {
        coder.lines[l] = (char*)malloc(MAX_LINE_LENGTH);
        int charIdx = 0;
        while (rawText[rawIdx] && rawText[rawIdx] != '\n' && rawText[rawIdx] != '\r') {
            if (charIdx < MAX_LINE_LENGTH - 1) {
                coder.lines[l][charIdx] = rawText[rawIdx];
                charIdx++;
            }
            rawIdx++;
        }
        coder.lines[l][charIdx] = '\0';
        if (rawText[rawIdx] == '\r') rawIdx++;
        if (rawText[rawIdx] == '\n') rawIdx++;
    }
    free(rawText);
    coder.cursorX = 0;
    coder.cursorY = 0;
}
void qCoderInputChar(char c) {
    char* currentLine = coder.lines[coder.cursorY];
    int len = qStrlen(currentLine);
    if (len >= MAX_LINE_LENGTH - 1) return;
    for (int i = len; i >= coder.cursorX; i--) { currentLine[i + 1] = currentLine[i]; }
    currentLine[coder.cursorX] = c;
    coder.cursorX++;
}
void qCoderSave() {
    char* bigBuffer = (char*)malloc(coder.lineCount * (MAX_LINE_LENGTH + 1));
    bigBuffer[0] = '\0';
    for (int i = 0; i < coder.lineCount; i++) { qStradd(bigBuffer, coder.lines[i]); if (i < coder.lineCount - 1) { qStradd(bigBuffer, "\n"); } }
    qWriteTextFile(coder.filename, bigBuffer);
    free(bigBuffer);
    int oldX = coder.cursorX;
    int oldY = coder.cursorY;
    qCoderOpenFile(coder.filename);
    coder.cursorX = oldX;
    coder.cursorY = oldY;
}

int actualWindow = 1;
int toSetWin = 0; void setWin() { actualWindow = toSetWin; }

void Init() {}
void Update() {
    // int w = getWidth() / 2, h = getHeight() / 2;
    if (onKey(QKEY_LEFT) && coder.cursorX > 0) coder.cursorX--;
    if (onKey(QKEY_RIGHT) && coder.cursorX < MAX_LINE_LENGTH) coder.cursorX++;
    if (onKey(QKEY_A)) { qCoderInputChar('a'); }

    updateUI(actualWindow);
    toSetWin = 0; drawToolButton("Viewport", actualWindow == 0, setWin);
    toSetWin = 1; drawToolButton("Coder", actualWindow == 1, setWin);
    toSetWin = 2; drawToolButton("Save", actualWindow == 2, setWin);
    toSetWin = 3; drawToolButton("Run", actualWindow == 3, setWin);
    toSetWin = 4; drawToolButton("Build", actualWindow == 4, setWin);
}
int main(int argc, char* argv[]) {
    if (argc >= 2) {
        char* tmp = (char*)malloc((size_t)qStrlen(argv[1])+16);
        if (!qExists(argv[1])) qCreateDir(argv[1]);
        qStrcpy(tmp, argv[1]); qStradd(tmp, "Assets/"); if (!qExists(tmp)) qCreateDir(tmp);
        qStrcpy(tmp, argv[1]); qStradd(tmp, "Temp/"); if (!qExists(tmp)) qCreateDir(tmp);
        qStrcpy(tmp, argv[1]); qStradd(tmp, ".qeproj"); if (!qExists(tmp)) qWriteTextFile(tmp, "editor: 0.1.0\nname: test");

        qStrcpy(tmp, argv[1]); qStradd(tmp, "Assets/test.c");
        qCoderOpenFile(tmp);
        free(tmp);
        print("Silnik QEngine uruchomiony z argumentem:\n %s", argv[1]);
        qgpuCreate(1280, 720, "QEngine v0.1.0 - test", Init, Update);
    } else { print("QEngine Error launch"); }
    return 0;
}
