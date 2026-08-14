#define QEngine_Input
#define QEngine_Math
// #define QEngine_Memory
// #define QEngine_IO
// #define QEngine_Text
#include "../Libs/QEngine.h"

void init() {
	print("Hello\n");
}
void update() {

}

int main() { return initEngineProject(init, update); }
