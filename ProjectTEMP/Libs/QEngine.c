#define QEngine_Input
#define QEngine_IO
#define QEngine_Math
#define QEngine_Memory
#define QEngine_Text
#include "../Assets/QEngine.h"

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
static Camera _camera;
int initEngineProject(void (*initFunc)(), void (*updateFunc)()) { initMem(); qgpuCreate(QEP_START_WIDTH, QEP_START_HEIGHT, QEP_NAME, initFunc, updateFunc); return 0; }

// = = = = = CAMERA = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
Camera getCamera() { return _camera; }
void setCamera(Camera camera) { _camera = camera; }
void setCameraPos(Vector3 position) { _camera.position = position; }
void setCameraRot(Vector3 rotation) { _camera.position = rotation; }
void setCameraScale(Vector3 scale) { _camera.position = scale; }

// = = = = = GRAPHIC = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


// = = = = = INPUT = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
int getKeyState(int keyCode) { return qgGetKey(keyCode); }
int onKeyDown(int keyCode) { return qgOnKey(keyCode); }
int getMouseButton(int mouseKey) { return qgGetMouse(mouseKey); }
int onMouseDown(int mouseKey) { return qgOnMouse(mouseKey); }
Vector2 getCursorPosition() { double x, y; qgGetMousePos(&x, &y); return (Vector2){x, y}; }
