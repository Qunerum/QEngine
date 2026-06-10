#include "../lib/qgpu.h"

void Init() {
}
void Update() {
}

int main() {
    qgpuCreate(1280, 720, "QEngine", Init, Update);
    return 0;
}
