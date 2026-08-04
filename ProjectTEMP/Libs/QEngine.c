#include <stdlib.h>
#include <stdio.h>

#define QEngine_Input
#define QEngine_Math
#define QEngine_Memory
#define QEngine_IO
#define QEngine_Text
#include "QEngine.h"

#include "qgpu.h"
#include "../Data/PROJECT.h"
#include <stdarg.h>

#define QENGINE_VERSION_MAJOR 0
#define QENGINE_VERSION_MINOR 1
#define QENGINE_VERSION_PATCH 1

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
	if (!buffer) { fclose(file); return NULL; }
	fread(buffer, 1, size, file);
	buffer[size] = '\0';
	fclose(file);
	return buffer;
}
int qWriteFileText(const char* filename, const char* text) {
	FILE* file = fopen(filename, "w");
	if (!file) return 0;
	int result = fputs(text, file);
	fclose(file);
	return result != EOF;
}
int qFileExists(const char* filename) {
	FILE* file = fopen(filename, "r");
	if (file) { fclose(file); return 1; }
	return 0;
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

static Camera _camera;
int initEngineProject(void (*initFunc)(), void (*updateFunc)()) {
	qgSetBackground(.1f, .1f, .1f);
	qgpuCreate(QEP_START_WIDTH, QEP_START_HEIGHT, QEP_NAME, initFunc, updateFunc);
	return 0;
}
void setDrawingMode(qeDrawingMode mode) {
	switch (mode) {
		case World: qgSetRenderType(QGPU_RENDER_TYPE_LIGHT); break;
		case UI: qgSetRenderType(QGPU_RENDER_TYPE_NO_LIGHT); break;
	}
}

void addObjectToPublic(QObject object) {
	//
}

// = = = = = CAMERA = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
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

void drawRect(Vector3 position, Vector3 rotation, Vector2 size, Color color) {
	setRot(position, rotation);
	qgAddRect(position.x, position.y, position.z, size.x, size.y, byteTo01(color.r), byteTo01(color.g), byteTo01(color.b), byteTo01(color.a));
}

void drawText(Vector3 position, Vector3 rotation, float fontSize, const char* text, Color color) {
	setRot(position, rotation);
	qgSetFontData(fontSize, QGPU_FONT_STYLE_REGULAR, color.r, color.g, color.b, color.a);
	qgAddText(position.x, position.y, position.z, text);
}

// = = = = = INPUT = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
int getKeyState(int keyCode) { return qgGetKey(keyCode); }
int onKeyDown(int keyCode) { return qgOnKey(keyCode); }
int getMouseButton(int mouseKey) { return qgGetMouse(mouseKey); }
int onMouseDown(int mouseKey) { return qgOnMouse(mouseKey); }
Vector2 getCursorPosition() { double x, y; qgGetMousePos(&x, &y); return (Vector2){x, y}; }
