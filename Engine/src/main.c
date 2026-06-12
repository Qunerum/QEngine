#include "../lib/qgpu.h"
#include "../include/ui.h"

void toolActionNothing() {}

void Init() {
    // loadTexture(FILES "test.qgt", 0);
}
int actualWindow = 0;
void Update() {
    // int w = getWidth() / 2, h = getHeight() / 2;
    updateUI();
    drawToolButton("Viewport", 0, toolActionNothing);
    drawToolButton("Coder", 1, toolActionNothing);
    drawToolButton("Save", 0, toolActionNothing);
    drawToolButton("Run", 0, toolActionNothing);
    drawToolButton("Build", 0, toolActionNothing);
    drawWindow(actualWindow);
}

int main() {
    qgpuCreate(1280, 720, "QEngine v0.1.0", Init, Update);
    return 0;
}
