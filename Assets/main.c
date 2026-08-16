#define QEngine_Audio
#define QEngine_Input
// #define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"

int s = 0;
void init() {
	convertAudio("Assets/sound.qsr", "Assets/sound.qs");
	s = loadAudio("Assets/sound.qs");
}
void update() {
	if (onKeyDown(KEY_SPACE)) {
		playAudio(s, 255, 1);
	}
}

int main() { return initEngineProject(init, update); }
