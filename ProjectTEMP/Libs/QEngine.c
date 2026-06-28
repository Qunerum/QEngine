#include "QEngine.h"
#include "QEngine.Memory.h"
#include "Dev/qgpu.h"
#include "../Data/PROJECT.h"

// = = = = = MEMORY = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// = = = = = = = = = = qMalloc , qFree
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
// = = = = = = = = = =
static Camera _camera;
// = = = = = = = = = = initEngineProject ,
int initEngineProject(void (*initFunc)(), void (*updateFunc)()) { initMem(); qgpuCreate(QEP_START_WIDTH, QEP_START_HEIGHT, QEP_NAME, initFunc, updateFunc); return 0; }



// = = = = = CAMERA = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// = = = = = = = = = = setCamera ,
Camera getCamera() { return _camera; }
void setCamera(Camera camera) { _camera = camera; }
void setCameraPos(Vector3 position) { _camera.position = position; }
void setCameraRot(Vector3 rotation) { _camera.position = rotation; }
void setCameraScale(Vector3 scale) { _camera.position = scale; }

// = = = = = GRAPHIC = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// = = = = = = = = = =



