#ifndef QENGINE_MAIN_H
#define QENGINE_MAIN_H
#include <stdint.h>
#include <stddef.h>

int initEngineProject(void (*initFunc)(), void (*updateFunc)());

// = = = = = STRUCTURES = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
typedef struct { float x, y; } Vector2;
typedef struct { int x, y; } Vector2Int;
typedef struct { float x, y, z; } Vector3;
typedef struct { int x, y, z; } Vector3Int;
typedef struct { Vector3 position, rotation, scale; } Transform;

typedef struct { Vector3 position, rotation; float fov; } Camera;
typedef struct { char* name; Transform transform; int isActive; } QObject;

void addObjectToPublic(QObject object);

#endif
