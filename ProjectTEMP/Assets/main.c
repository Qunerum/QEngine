#define QEngine_Text
#include "../Libs/QEngine.h"

void init() {
	char t[] = "AbCdE";
	qReverse(t);
	qgPrint("%s\n", t);
}
void update() {

}

int main() { return initEngineProject(init, update); }
