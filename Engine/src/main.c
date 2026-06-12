#include "../lib/qgpu.h"
#include "../include/ui.h"

void toolActionNothing() {}

void Init() {
    // loadTexture(FILES "test.qgt", 0);
}
void Update() {
    // int w = getWidth() / 2, h = getHeight() / 2;
    updateUI();
    drawToolButton("Engine", toolActionNothing);
    drawToolButton("Save", toolActionNothing);
    drawToolButton("Run", toolActionNothing);
    drawToolButton("Build", toolActionNothing);
}

int main() {
    qgpuCreate(1920, 1080, "QEngine v0.1.0", Init, Update);
    return 0;
}
