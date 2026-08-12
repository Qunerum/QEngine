#define QEngine_Input
#define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"
#include "../Libs/qgpu.h"

static state isLight = false;
float c(float v) { return v + isLight * 0.4f; }

static int w = 0, h = 0,
/*objC = 0,*/ objListW = 240;
// void drawObjOnList(QObject obj) {
	//
	// objC++;
// }
void drawEngineUI() {
	// Object list
	qgAddRect(-w / 2.f + objListW / 2.f, 0, 0, objListW, h, c(.2f), c(.2f), c(.2f), 1);
	// Manager
	// drawRect();
}


void init() {
	print("Application was made in QEngine v%i.%i.%i\n", QENGINE_VERSION_MAJOR, QENGINE_VERSION_MINOR, QENGINE_VERSION_PATCH);
	setDrawingMode(UI);
}
void update() {
	w = qgGetWidth();
	h = qgGetHeight();
	drawEngineUI();
}

int main() { return initEngineProject(init, update); }
