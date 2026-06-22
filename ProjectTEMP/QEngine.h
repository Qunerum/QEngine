#ifndef QENGINE_MAIN_H
#define QENGINE_MAIN_H

int initEngineProject(void (*initFunc)(), void (*updateFunc)());

typedef struct { float x, y; } Vector2;
typedef struct { int x, y; } Vector2Int;
typedef struct { float x, y, z; } Vector3;
typedef struct { int x, y, z; } Vector3Int;

static inline float qLerp_f(float A, float B, float t) { return A + t * (B - A); }
static inline Vector2 qLerp_v2(Vector2 A, Vector2 B, float t) { return (Vector2){A.x + t * (B.x - A.x), A.y + t * (B.y - A.y)}; }
static inline Vector3 qLerp_v3(Vector3 A, Vector3 B, float t) { return (Vector3){A.x + t * (B.x - A.x), A.y + t * (B.y - A.y), A.z + t * (B.z - A.z)}; }
#define qLerp(A, B, t) _Generic((A), float:qLerp_f, Vector2:qLerp_v2, Vector3:qLerp_v3 )(A, B, t)

static inline int qClamp_i(int v, int min, int max) { return v > max ? max : v < min ? min : v; }
static inline float qClamp_f(float v, float min, float max) { return v > max ? max : v < min ? min : v; }
#define qClamp(Value, Min, Max) _Generic((Value), int:qClamp_i, float:qClamp_f )(Value, Min, Max)

#endif
