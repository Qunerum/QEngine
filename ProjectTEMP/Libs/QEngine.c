#define QEngine_Input
#define QEngine_IO
#define QEngine_Math
#define QEngine_Memory
#define QEngine_Text
#include "QEngine.h"

#include "qgpu.h"
#include "../Data/PROJECT.h"
#include <stdarg.h>

// = = = = = MEMORY = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static char heap_memory[HEAP_SIZE]; static MemoryBlock* freeList = (MemoryBlock*)heap_memory; static int qMemoryInited = 0;
void initMem() { if (qMemoryInited) return; qMemoryInited = 1; freeList->size = HEAP_SIZE - sizeof(MemoryBlock); freeList->free = 1; freeList->next = 0; }
void* qMalloc(size_t size) { size = ALIGN(size); MemoryBlock* curr = freeList; while (curr) { if (curr->free && curr->size >= size) { if (curr->size > size + sizeof(MemoryBlock) + 8) {
	MemoryBlock* nextBlock = (MemoryBlock*)((char*)curr + sizeof(MemoryBlock) + size); nextBlock->size = curr->size - size - sizeof(MemoryBlock);
	nextBlock->free = 1; nextBlock->next = curr->next; curr->size = size; curr->next = nextBlock; }
	curr->free = 0; unsigned char* p = (unsigned char*)((char*)curr + sizeof(MemoryBlock)); for(size_t i = 0; i < size; i++) p[i] = 0; return (void*)p; }
	curr = curr->next; } return 0; /* Out of memory! */ }
void qFree(void* ptr) { if (!ptr) return; MemoryBlock* block = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock)); block->free = 1; MemoryBlock* curr = freeList;
	while (curr && curr->next) { if (curr->free && curr->next->free) { curr->size += curr->next->size + sizeof(MemoryBlock); curr->next = curr->next->next; } else { curr = curr->next; } } }
// = = = = = ENGINE = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
extern void vprintc(int color, const char* format, va_list args);
void print(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vprintc(244, format, args);
	va_end(args);
}

static Camera _camera;
int initEngineProject(void (*initFunc)(), void (*updateFunc)()) {
	initMem();
	qgSetBackground(.1f, .1f, .1f);
	qgpuCreate(QEP_START_WIDTH, QEP_START_HEIGHT, QEP_NAME, initFunc, updateFunc);
	return 0;
}

// = = = = = CAMERA = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
Camera getCamera() { return _camera; }
void setCamera(Camera camera) { _camera = camera; }
void setCameraPos(Vector3 position) { _camera.position = position; }
void setCameraRot(Vector3 rotation) { _camera.position = rotation; }
void setCameraScale(Vector3 scale) { _camera.position = scale; }

// = = = = = GRAPHIC = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static float byteTo01(byte v) { return v / 255.0f; }
void drawRect(Vector3 position, Vector3 rotation, Vector2 size, Color color) {
	float px = position.x, py = position.y, pz = position.z,
	x = size.x / 2, y = size.y / 2, r = byteTo01(color.r), g = byteTo01(color.g), b = byteTo01(color.b), a = byteTo01(color.a);
	uint32_t v1 = qgAddVertex(px-x, py+y, pz, r, g, b, a), v2 = qgAddVertex(px+x, py+y, pz, r, g, b, a),
	v3 = qgAddVertex(px-x, py-y, pz, r, g, b, a), v4 = qgAddVertex(px+x, py-y, pz, r, g, b, a);
	qgAddIndex(v1); qgAddIndex(v2); qgAddIndex(v4);
	qgAddIndex(v1); qgAddIndex(v4); qgAddIndex(v3);
}

// = = = = = INPUT = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
int getKeyState(int keyCode) { return qgGetKey(keyCode); }
int onKeyDown(int keyCode) { return qgOnKey(keyCode); }
int getMouseButton(int mouseKey) { return qgGetMouse(mouseKey); }
int onMouseDown(int mouseKey) { return qgOnMouse(mouseKey); }
Vector2 getCursorPosition() { double x, y; qgGetMousePos(&x, &y); return (Vector2){x, y}; }
