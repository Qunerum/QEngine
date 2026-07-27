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
	static float x = 0, speed = 1;
	static int toLeft = 0;
	if (x > 100) toLeft = 1;
	if (x < -100) toLeft = 0;
	x += toLeft ? -speed : speed;

	if (getKeyState(KEY_W) && speed < 20) speed += 0.1f;
	if (getKeyState(KEY_S) && speed >= 0.1f) speed -= 0.1f;

	static char info[32];
	formatText(info, sizeof(info), "Speed: %.1f", speed);

	drawText(Vector3_Mul(Vector3_Up, -60), Vector3_Zero, 2, info, Color_White);

	drawRect((Vector3){x, 0, 0}, Vector3_Zero, Vector2_Mul(Vector2_One, 100), Color_Blue);
}

int main() { return initEngineProject(init, update); }
