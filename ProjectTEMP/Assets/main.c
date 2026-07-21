#define QEngine_Input
#define QEngine_Text
#include "../Libs/QEngine.h"

void init() {
	char t[] = "AbCdE";
	qReverse(t);
	print("%s\n", t);
}
Color clrs[] = {
	Color_Light_Gray,
	Color_Light_Red,
	Color_Light_Green,
	Color_Light_Yellow,
	Color_Light_Orange,
	Color_Light_Blue,
	Color_Light_Magenta,
	Color_Light_Cyan,

	Color_Gray,
	Color_Red,
	Color_Green,
	Color_Yellow,
	Color_Orange,
	Color_Blue,
	Color_Magenta,
	Color_Cyan,

	Color_Dark_Gray,
	Color_Dark_Red,
	Color_Dark_Green,
	Color_Dark_Yellow,
	Color_Dark_Orange,
	Color_Dark_Blue,
	Color_Dark_Magenta,
	Color_Dark_Cyan,
};
void update() {
	for (int i = 0; i < 8; i++) {
		drawRect((Vector3){-40, 100-(i*40), 0}, Vector3_Zero, (Vector2){40, 40}, clrs[i]);
		drawRect((Vector3){0, 100-(i*40), 0}, Vector3_Zero, (Vector2){40, 40}, clrs[i+8]);
		drawRect((Vector3){40, 100-(i*40), 0}, Vector3_Zero, (Vector2){40, 40}, clrs[i+16]);
	}
}

int main() { return initEngineProject(init, update); }
