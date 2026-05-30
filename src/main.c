#include "../lib/qgpu.h"

QColor normalUI = {0.4f, 0.4f, 0.4f, 1.0f};
QColor normalBtn  = {0.5f, 0.5f, 0.5f, 1.0f};
QColor press  = {0.3f, 0.3f, 0.3f, 1.0f};
void Update() {
    float w = getWidth(), h = getHeight();
    drawRect(0, -h / 2 + 50, w, 100, normalUI);
}

int main() {
    // loadTexture(FILES "test.qgt", 0);
    qgpuCreate(1280, 720, "QEngine", Update);
    return 0;
}
