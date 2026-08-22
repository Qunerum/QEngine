// #define QEngine_Audio
#define QEngine_Input
// #define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"

void init() {
}
void update() {

	Vector3 lp = V3(75, 20, 75);
	addLight(lp, 200, 1);
	drawSphere(lp, V3_Zero, 5, 5, 10, Color_Yellow);

	lp = V3(-75, -20, 75);
	addLight(lp, 300, 1);
	drawSphere(lp, V3_Zero, 5, 5, 10, Color_Yellow);

	static Vector3 rot = V3_Zero;
	const float spd = 0.8f;
	rot.x += spd;
	rot.y += spd;
	rot.z += spd;
	// drawBox(V3_Zero, rot, V3(100, 100, 100), Color_White);
	drawSphere(V3_Zero, rot, 32, 32, 75, Color_White);
}

int main() { return initEngineProject(init, update); }
