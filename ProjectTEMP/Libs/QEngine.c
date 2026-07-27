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
#define QENGINE_VERSION_PATCH 0

// = = = = = MEMORY = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static char heap_memory[HEAP_SIZE]; static MemoryBlock* freeList = (MemoryBlock*)heap_memory; static int qMemoryInited = 0;
static void initMem() { if (qMemoryInited) return; qMemoryInited = 1; freeList->size = HEAP_SIZE - sizeof(MemoryBlock); freeList->free = 1; freeList->next = 0; }
void* qMalloc(size_t size) { size = ALIGN(size); MemoryBlock* curr = freeList; while (curr) { if (curr->free && curr->size >= size) { if (curr->size > size + sizeof(MemoryBlock) + 8) {
	MemoryBlock* nextBlock = (MemoryBlock*)((char*)curr + sizeof(MemoryBlock) + size); nextBlock->size = curr->size - size - sizeof(MemoryBlock);
	nextBlock->free = 1; nextBlock->next = curr->next; curr->size = size; curr->next = nextBlock; }
	curr->free = 0; unsigned char* p = (unsigned char*)((char*)curr + sizeof(MemoryBlock)); for(size_t i = 0; i < size; i++) p[i] = 0; return (void*)p; }
	curr = curr->next; } return 0; /* Out of memory! */ }
void qFree(void* ptr) { if (!ptr) return; MemoryBlock* block = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock)); block->free = 1; MemoryBlock* curr = freeList;
	while (curr && curr->next) { if (curr->free && curr->next->free) { curr->size += curr->next->size + sizeof(MemoryBlock); curr->next = curr->next->next; } else { curr = curr->next; } } }
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
	initMem();
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
