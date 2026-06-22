#include "QEngine.h"
#include "Libs/qgpu.h"
#include "Data/PROJECT.h"

int initEngineProject(void (*initFunc)(), void (*updateFunc)()) {
	qgpuCreate(QEP_START_WIDTH, QEP_START_HEIGHT, QEP_NAME, initFunc, updateFunc);
	return 0;
}
