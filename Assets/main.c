#define QEngine_Audio
#define QEngine_Input
// #define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"

int s = 0, x = 0;
char t[33];
void init() {
	convertAudio("Assets/sound.qsr", "Assets/sound.qs");
	s = loadAudio("Assets/sound.qs");
	setDrawingMode(UI);
	enableInput();
}
void update() {
	if (onKeyDown(KEY_SPACE)) playAudio(s, 255, 1);
	getInput(t, 33);
	drawText(t, V3_Zero, V3_Zero, 1.6f, Center, Color_White);
}

int main() { return initEngineProject(init, update); }
