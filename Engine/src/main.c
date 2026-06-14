#include "../lib/qgpu.h"
#include "../include/main.h"
#include "../include/ui.h"
#include "../include/io.h"
#include "../include/text.h"
#include <stdlib.h>

char coderOpenedFile[MAX_FILE_NAME_LEN];
char coderBuffer[MAX_FILE_SIZE];
int coderBufferLen = 0, coderCursor = 0;
void qCoderOpenFile(char* filename) { char* rawText = qReadTextFile(filename); if (rawText) qStrcpy(coderBuffer, rawText); free(rawText); print(coderBuffer); }
void qCoderInputChar(char c) {
    for (int i = coderBufferLen; i >= coderCursor; i--) { coderBuffer[i + 1] = coderBuffer[i]; }
    coderBuffer[coderCursor] = c;
}
void qCoderSave() {
    qWriteTextFile(coderOpenedFile, coderBuffer);
    qCoderOpenFile(coderOpenedFile);
}

int actualWindow = 1;
int toSetWin = 0; void setWin() { actualWindow = toSetWin; }

void Init() {}
void Update() {
    // int w = getWidth() / 2, h = getHeight() / 2;
    // if (onKey(QKEY_UP) && coder.cursorY > 0) coder.cursorY--;
    // if (onKey(QKEY_DOWN) && coder.cursorY < coder.maxLines) coder.cursorY++;
    if (onKey(QKEY_LEFT) && coderCursor > 0) coderCursor--;
    if (onKey(QKEY_RIGHT) && coderCursor < MAX_FILE_SIZE - 1) coderCursor++;

    if (onKey(QKEY_ENTER)) { qCoderInputChar('\n'); }
    if (onKey(QKEY_A)) { qCoderInputChar('a'); }
    if (getKey(QKEY_LCTRL) && onKey(QKEY_S)) { print("Saving..."); qCoderSave(); }

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
