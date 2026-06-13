#include "../lib/qgpu.h"
#include "../include/ui.h"
#include "../include/io.h"
#include "../include/text.h"
#include <stdlib.h>

void toolActionNothing() {}
void Init() {
    // loadTexture(FILES "test.qgt", 0);
}
int actualWindow = 1;
void Update() {
    // int w = getWidth() / 2, h = getHeight() / 2;
    updateUI(actualWindow);
    drawToolButton("Viewport", 0, toolActionNothing);
    drawToolButton("Coder", 1, toolActionNothing);
    drawToolButton("Save", 0, toolActionNothing);
    drawToolButton("Run", 0, toolActionNothing);
    drawToolButton("Build", 0, toolActionNothing);
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
    char* tmp = (char*)malloc((size_t)qStrlen(argv[1])+16);
    if (!qExists(argv[1])) qCreateDir(argv[1]);
        qStrcpy(tmp, argv[1]); qStradd(tmp, "Assets/"); if (!qExists(tmp)) qCreateDir(tmp);
        qStrcpy(tmp, argv[1]); qStradd(tmp, "Temp/"); if (!qExists(tmp)) qCreateDir(tmp);
        qStrcpy(tmp, argv[1]); qStradd(tmp, ".qeproj"); if (!qExists(tmp)) qWriteTextFile(tmp, "editor: 0.1.0\nname: test");

        qStrcpy(tmp, argv[1]); qStradd(tmp, "Assets/test.c");
        char* fileData = qReadTextFile(tmp);
        free(tmp);
        if (fileData) setCoderData(fileData);
        print("Silnik QEngine uruchomiony z argumentem:\n %s", argv[1]);
        // load_project(argv[1]);
        qgpuCreate(1280, 720, "QEngine v0.1.0 - test", Init, Update);
        // free(fileData);
    } else { print("QEngine Error launch"); }
    return 0;
}
