#define QEngine_Input
#define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"
#include "../Libs/qgpu.h"

static state isLight = false;
static Color c1;

static int w = 0, h = 0,
objectListWidth = 300,
managerWidth = 300;
void drawEngineUI() {
	// Object list
	drawRect(Vector3(objectListWidth / 2.0f), Vector3_Zero, Vector2(objectListWidth, h), Left, c1);
	// Manager
	drawRect(Vector3(-managerWidth / 2.0f), Vector3_Zero, Vector2(managerWidth, h), Right, c1);
}


void init() {
	print("Application was made in QEngine v%i.%i.%i\n", QENGINE_VERSION_MAJOR, QENGINE_VERSION_MINOR, QENGINE_VERSION_PATCH);
	setDrawingMode(UI);

	c1 = Color(isLight ? 163 : 51);
}
void update() {
	w = qgGetWidth();
	h = qgGetHeight();
	drawEngineUI();
}

int main() { return initEngineProject(init, update); }
