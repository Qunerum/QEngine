// #define QEngine_Audio
#define QEngine_Input
// #define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"

char t[33];
void init() {
	setDrawingMode(UI);
	enableInput();
	setMaxInput(32);
}
void update() {
	getInput(t, sizeof(t));
	drawText(t, V3_Zero, V3_Zero, 1.6f, Center, Color_White);
}

int main() { return initEngineProject(init, update); }
