#define QEngine_Input
#define QEngine_Math
#define QEngine_Memory
#define QEngine_IO
#define QEngine_Text
#include "QEngine.h"

#include "qgpu.h"
#include "../Data/PROJECT.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
// = = = = = MEMORY = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void *qMalloc(size_t size) { return malloc(size); }
void *qRealloc(void *ptr, size_t size) { return realloc(ptr, size); }
void qFree(void *ptr) { free(ptr); }
// = = = = = IO = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
char* qReadFileText(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file) return NULL;
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* buffer = (char*)qMalloc(size + 1);
	if (!buffer) {
		fclose(file);
		return NULL;
	}
	fread(buffer, 1, size, file);
	buffer[size] = '\0';
	fclose(file);
	return buffer;
}
state qWriteFileText(const char* filename, const char* text) {
	FILE* file = fopen(filename, "w");
	if (!file) return false;
	int result = fputs(text, file);
	fclose(file);
	return result != EOF;
}
state qFileExists(const char* filename) {
	FILE* file = fopen(filename, "r");
	if (file) {
		fclose(file);
		return true;
	}
	return false;
}
// . . .
state qPathExists(const char* path) {
	struct stat st;
	if (stat(path, &st) == 0) return true;
	return false;
}
state qCopyFile(const char* srcPath, const char* dstPath) {
	FILE* src = fopen(srcPath, "rb");
	if (!src) return false;
	FILE* dst = fopen(dstPath, "wb");
	if (!dst) {
		fclose(src);
		return false;
	}
	char buffer[8192];
	size_t bytesRead;
	while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) fwrite(buffer, 1, bytesRead, dst);
	fclose(src);
	fclose(dst);
	return true;
}
// = = = = = ENGINE = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
extern void qgVprintc(int color, const char* format, va_list args);
void print(const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(244, format, args);
	va_end(args);
}
int formatText(char* to, int length, const char* format, ...) {
	va_list args;
	va_start(args, format);
	int written = vsnprintf(to, length, format, args);
	va_end(args);
	return written;
}
int initEngineProject(void (*initFunc)(), void (*updateFunc)()) {
	qgSetBackground(0, 0, 0);
	qgpuCreate(QEP_START_WIDTH, QEP_START_HEIGHT, QEP_NAME, initFunc, updateFunc);
	return 0;
}
// = = = = = CAMERA = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static Camera _camera;
void setDrawingMode(qeDrawingMode mode) {
	switch (mode) {
		case World: qgSetRenderType(QGPU_RENDER_TYPE_LIGHT); break;
		case UI: qgSetRenderType(QGPU_RENDER_TYPE_NO_LIGHT); break;
	}
}
Camera getCamera() { return _camera; }
void setCamera(Camera camera) { _camera = camera; }
void setCameraPos(Vector3 position) { _camera.position = position; }
void setCameraRot(Vector3 rotation) { _camera.position = rotation; }
void setCameraScale(Vector3 scale) { _camera.position = scale; }
// = = = = = GRAPHIC = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static float byteTo01(byte v) { return v / 255.0f; }
static void setRot(Vector3 position, Vector3 rotation) {
	qgSetRotationPivot(position.x, position.y, position.z);
	qgSetRotation(rotation.x, rotation.y, rotation.z);
}
// = = = = = INPUT = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
state getKeyState(int keyCode) { return qgGetKey(keyCode); }
state onKeyDown(int keyCode) { return qgOnKey(keyCode); }
state getMouseButton(int mouseKey) { return qgGetMouse(mouseKey); }
state onMouseDown(int mouseKey) { return qgOnMouse(mouseKey); }
Vector2 getCursorPosition() {
	double x, y;
	qgGetMousePos(&x, &y);
	return (Vector2){x, y};
}
