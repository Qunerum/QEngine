#define QEngine_Input
#define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"
#include "../Libs/qgpu.h"

static state isLight = false;
float c(float v) { return v + isLight * 0.4f; }

static int objListW = 300;
void drawEngineUI(int w, int h) {
	// Object list
	qgAddRect(-w / 2.f + objListW / 2.f, 0, 0, objListW, h, c(.2f), c(.2f), c(.2f), 1);
}


void init() {
	print("Application was made in QEngine v%i.%i.%i\n", QENGINE_VERSION_MAJOR, QENGINE_VERSION_MINOR, QENGINE_VERSION_PATCH);
	setDrawingMode(UI);
}
void update() {
	int w = qgGetWidth(), h = qgGetHeight();
	drawEngineUI(w, h);
}

int main() { return initEngineProject(init, update); }
