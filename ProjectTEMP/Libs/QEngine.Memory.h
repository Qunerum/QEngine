#ifndef QENGINE_LIB_MEMORY_H
#define QENGINE_LIB_MEMORY_H
#include "QEngine.h"

#define HEAP_SIZE  0x1000000 // 16MB
#define ALIGN(size) (((size) + 7) & ~7)
typedef struct MemoryBlock { size_t size; int free; struct MemoryBlock* next; } MemoryBlock;

void* qMalloc(size_t size);
void qFree(void* ptr);

#endif
