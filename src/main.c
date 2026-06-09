#include "../lib/qgpu.h"

float s = 30;
void Init() {
    loadTexture(FILES "test.qgt", 0);
}
void Update() {
    if (getKey(QKEY_UP)) s++;
    if (getKey(QKEY_DOWN) && s > 0) s--;

    drawTextureScale(0, 0, 0, s);
}

int main() {
    qgpuCreate(1280, 720, "QGPU Template Project", Init, Update);
    return 0;
}
