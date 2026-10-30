// #define QEngine_Audio
#define QEngine_Input
#define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"

void init() { }
void update() {
	addPointLight(V3(2, 0, 2), 50, 1, Color_White);
	static float r = 0.0f;
	r += 0.5f;
	drawBox(V3_Zero, V3(30, r, 0), V3(1, 0.5f, 1), Color_Red);
	drawBox(V3(0, -1, 0), V3_Zero, V3(10, 0.1f, 10), Color_White);

	const float spd = 0.25f;
	static Vector3 cam = V3_Zero;
	const Vector2 delta = Vector2_Mul(getMouseDelta(), spd);
	cam.x = qClamp(cam.x + delta.y, -80.0f, 80.0f);
	cam.y -= delta.x;
	setCameraRot(cam);

	static state isLocked = false;
	if (onKeyDown(KEY_ESCAPE)) {
		isLocked = !isLocked;
		setCursorState(isLocked ? CURSOR_DISABLED : CURSOR_NORMAL);
	}
}
const Scene mainScene = {init, update};
int main() { return initEngineProject(mainScene); }
