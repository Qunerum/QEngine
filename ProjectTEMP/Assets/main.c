#define QEngine_Input
#define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"

// Executed exactly once at start of the program.
void init() {
	print("Hello, World!\n");
	setDrawingMode(UI);
}

// Executed continuosly on every frame.
void update() {
}

int main() { return initEngineProject(init, update); }
