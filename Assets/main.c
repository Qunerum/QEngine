// #define QEngine_Audio
// #define QEngine_Input
// #define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"

void init() { }
void update() {
	addLight(V3(2, 0, 2), 50, 1);
	static float r = 0;
	r += 0.5f;
	drawBox(V3_Zero, V3(30, r, 0), V3(1, 0.5f, 1), Color_Red);
	drawBox(V3(0, -1, 0), V3_Zero, V3(10, 0.1f, 10), Color_White);
}
const Scene mainScene = {init, update};
int main() { return initEngineProject(mainScene); }
