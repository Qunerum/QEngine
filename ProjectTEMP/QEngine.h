#ifndef QENGINE_MAIN_H
#define QENGINE_MAIN_H

int initEngineProject(void (*initFunc)(), void (*updateFunc)());

typedef struct { float x, y; } Vector2;
typedef struct { int x, y; } Vector2Int;
typedef struct { float x, y, z; } Vector3;
typedef struct { int x, y, z; } Vector3Int;

// = = = = = LERP = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qLerp_f(float A, float B, float t) { return A + t * (B - A); }
static inline Vector2 qLerp_v2(Vector2 A, Vector2 B, float t) { return (Vector2){qLerp_f(A.x, B.x, t), qLerp_f(A.y, B.y, t)}; }
static inline Vector3 qLerp_v3(Vector3 A, Vector3 B, float t) { return (Vector3){qLerp_f(A.x, B.x, t), qLerp_f(A.y, B.y, t), qLerp_f(A.z, B.z, t)}; }
#define qLerp(A, B, t) _Generic((A), float:qLerp_f, Vector2:qLerp_v2, Vector3:qLerp_v3 )(A, B, t)

// = = = = = CLAMP = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qClamp_i(int v, int min, int max) { return v > max ? max : v < min ? min : v; }
static inline float qClamp_f(float v, float min, float max) { return v > max ? max : v < min ? min : v; }
static inline Vector2 qClamp_v2(Vector2 v, Vector2 min, Vector2 max) { return (Vector2){qClamp_f(v.x, min.x, max.x), qClamp_f(v.y, min.y, max.y)}; }
static inline Vector2Int qClamp_v2i(Vector2Int v, Vector2Int min, Vector2Int max) { return (Vector2Int){qClamp_i(v.x, min.x, max.x), qClamp_i(v.y, min.y, max.y)}; }
static inline Vector3 qClamp_v3(Vector3 v, Vector3 min, Vector3 max) { return (Vector3){qClamp_f(v.x, min.x, max.x), qClamp_f(v.y, min.y, max.y), qClamp_f(v.z, min.z, max.z)}; }
static inline Vector3Int qClamp_v3i(Vector3Int v, Vector3Int min, Vector3Int max) { return (Vector3Int){qClamp_i(v.x, min.x, max.x), qClamp_i(v.y, min.y, max.y), qClamp_i(v.z, min.z, max.z)}; }
#define qClamp(Value, Min, Max) _Generic((Value), int:qClamp_i, float:qClamp_f, Vector2:qClamp_v2, Vector2Int:qClamp_v2i, Vector3:qClamp_v3, Vector3Int:qClamp_v3i )(Value, Min, Max)

// = = = = = MAP = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qMap_f(float v, float oldMin, float oldMax, float newMin, float newMax) {
	if (oldMax - oldMin == 0.0f) return newMin; float pct = (v - oldMin) / (oldMax - oldMin); return qLerp_f(newMin, newMax, pct); }
static inline Vector2 qMap_v2(Vector2 v, Vector2 oldMin, Vector2 oldMax, Vector2 newMin, Vector2 newMax) {
	return (Vector2){qMap_f(v.x, oldMin.x, oldMax.x, newMin.x, newMax.x), qMap_f(v.y, oldMin.y, oldMax.y, newMin.y, newMax.y)}; }
	static inline Vector3 qMap_v3(Vector3 v, Vector3 oldMin, Vector3 oldMax, Vector3 newMin, Vector3 newMax) {
		return (Vector3){qMap_f(v.x, oldMin.x, oldMax.x, newMin.x, newMax.x), qMap_f(v.y, oldMin.y, oldMax.y, newMin.y, newMax.y), qMap_f(v.z, oldMin.z, oldMax.z, newMin.z, newMax.z)}; }
#define qMap(Value, oldMin, oldMax, newMin, newMax) _Generic((Value), float:qMap_f, Vector2:qMap_v2, Vector3:qMap_v3 )(Value, oldMin, oldMax, newMin, newMax)

// = = = = = MATH = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qPow(float base, int exp) { if (exp == 0) return 1; float v = base; int absExp = exp < 0 ? -exp : exp; for (int i=1;i<absExp;i++) { v *= base; } if (exp < 0) v = 1.0f/v; return v; }
static inline float qSqrt(float number) {	if (number <= 0.0f) return 0.0f; float x = number * 0.5f; for (int i=0;i<4;i++) x = 0.5f * (x + number / x); return x; }

static inline float qDist_v2(Vector2 a, Vector2 b) { return qSqrt(qPow(b.x - a.x, 2) + qPow(b.y - a.y, 2)); }
static inline float qDist_v3(Vector3 a, Vector3 b) { return qSqrt(qPow(b.x - a.x, 2) + qPow(b.y - a.y, 2) + qPow(b.z - a.z, 2)); }
#define qDist(A, B) _Generic((A), Vector2:qDist_v2, Vector3:qDist_v3 )(A, B)

#endif
