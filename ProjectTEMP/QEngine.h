#ifndef QENGINE_MAIN_H
#define QENGINE_MAIN_H

int initEngineProject(void (*initFunc)(), void (*updateFunc)());

// = = = = = STRUCTURES = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
typedef struct { float x, y; } Vector2;
typedef struct { int x, y; } Vector2Int;
typedef struct { float x, y, z; } Vector3;
typedef struct { int x, y, z; } Vector3Int;

#endif
