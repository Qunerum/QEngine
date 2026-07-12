#include "QEngine.h"
#include "QEngine.Text.h"

void init() {
	char* t = "AbCdE";
	qReverse(t);
	qPrint(t);
}
void update() {

}

int main() { return initEngineProject(init, update); }
