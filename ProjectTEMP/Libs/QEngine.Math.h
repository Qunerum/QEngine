#ifndef QENGINE_LIB_MATH_H
#define QENGINE_LIB_MATH_H
#include "QEngine.h"

#define PI 3.14159265358979323846f
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
static inline float qMap_f(float v, float oldMin, float oldMax, float newMin, float newMax) { if (oldMax - oldMin == 0.0f) return newMin; float pct = (v - oldMin) / (oldMax - oldMin); return qLerp_f(newMin, newMax, pct); }
static inline Vector2 qMap_v2(Vector2 v, Vector2 oldMin, Vector2 oldMax, Vector2 newMin, Vector2 newMax) {
	return (Vector2){qMap_f(v.x, oldMin.x, oldMax.x, newMin.x, newMax.x), qMap_f(v.y, oldMin.y, oldMax.y, newMin.y, newMax.y)}; }
static inline Vector3 qMap_v3(Vector3 v, Vector3 oldMin, Vector3 oldMax, Vector3 newMin, Vector3 newMax) {
	return (Vector3){qMap_f(v.x, oldMin.x, oldMax.x, newMin.x, newMax.x), qMap_f(v.y, oldMin.y, oldMax.y, newMin.y, newMax.y), qMap_f(v.z, oldMin.z, oldMax.z, newMin.z, newMax.z)}; }
#define qMap(Value, oldMin, oldMax, newMin, newMax) _Generic((Value), float:qMap_f, Vector2:qMap_v2, Vector3:qMap_v3 )(Value, oldMin, oldMax, newMin, newMax)
// = = = = = ABS (ABSOLUTE) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qAbs_i(int v) { return v < 0 ? -v : v; }
static inline float qAbs_f(float v) { return v < 0.0f ? -v : v; }
static inline Vector2 qAbs_v2(Vector2 v) { return (Vector2){qAbs_f(v.x), qAbs_f(v.y)}; }
static inline Vector2Int qAbs_v2i(Vector2Int v) { return (Vector2Int){qAbs_i(v.x), qAbs_i(v.y)}; }
static inline Vector3 qAbs_v3(Vector3 v) { return (Vector3){qAbs_f(v.x), qAbs_f(v.y), qAbs_f(v.z)}; }
static inline Vector3Int qAbs_v3i(Vector3Int v) { return (Vector3Int){qAbs_i(v.x), qAbs_i(v.y), qAbs_i(v.z)}; }
#define qAbs(V) _Generic((V), int:qAbs_i, float:qAbs_f, Vector2:qAbs_v2, Vector2Int:qAbs_v2i, Vector3:qAbs_v3, Vector3Int:qAbs_v3i )(V)
// = = = = = SIGN = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qSign_i(int v) { return (v > 0) - (v < 0); }
static inline float qSign_f(float v) { return (float)((v > 0.0f) - (v < 0.0f)); }
static inline Vector2 qSign_v2(Vector2 v) { return (Vector2){qSign_f(v.x), qSign_f(v.y)}; }
static inline Vector2Int qSign_v2i(Vector2Int v) { return (Vector2Int){qSign_i(v.x), qSign_i(v.y)}; }
static inline Vector3 qSign_v3(Vector3 v) { return (Vector3){qSign_f(v.x), qSign_f(v.y), qSign_f(v.z)}; }
static inline Vector3Int qSign_v3i(Vector3Int v) { return (Vector3Int){qSign_i(v.x), qSign_i(v.y), qSign_i(v.z)}; }
#define qSign(V) _Generic((V), int:qSign_i, float:qSign_f, Vector2:qSign_v2, Vector2Int:qSign_v2i, Vector3:qSign_v3, Vector3Int:qSign_v3i )(V)
// = = = = = ROUNDING = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qFloor(float v) { int i = (int)v; return v < i ? i - 1 : i; }
static inline int qCeil(float v) { int i = (int)v; return v > i ? i + 1 : i; }
static inline int qRound(float v) { return v < 0.0f ? (int)(v - 0.5f) : (int)(v + 0.5f); }
// = = = = = POW (POWER) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qPow(float base, int exp) { if (exp == 0) return 1; float v = base; int absExp = exp < 0 ? -exp : exp; for (int i=1;i<absExp;i++) { v *= base; } if (exp < 0) v = 1.0f/v; return v; }
// = = = = = SQRT (SQUARE ROOT) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qSqrt(float number) {	if (number <= 0.0f) return 0.0f; float x = number * 0.5f; for (int i=0;i<4;i++) x = 0.5f * (x + number / x); return x; }
// = = = = = DIST (DISTANCE) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qDist_v2(Vector2 a, Vector2 b) { return qSqrt(qPow(b.x - a.x, 2) + qPow(b.y - a.y, 2)); }
static inline float qDist_v3(Vector3 a, Vector3 b) { return qSqrt(qPow(b.x - a.x, 2) + qPow(b.y - a.y, 2) + qPow(b.z - a.z, 2)); }
#define qDist(A, B) _Generic((A), Vector2:qDist_v2, Vector3:qDist_v3 )(A, B)
// = = = = = LENGTH = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qLength_v2(Vector2 v) { return qSqrt(v.x * v.x + v.y * v.y); }
static inline float qLength_v3(Vector3 v) { return qSqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
#define qLength(V) _Generic((V), Vector2:qLength_v2, Vector3:qLength_v3 )(V)
// = = = = = NORMALIZE = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline Vector2 qNormalize_v2(Vector2 v) { float len = qLength_v2(v); if (len == 0.0f) return (Vector2){0.0f, 0.0f}; return (Vector2){v.x / len, v.y / len}; }
static inline Vector3 qNormalize_v3(Vector3 v) { float len = qLength_v3(v); if (len == 0.0f) return (Vector3){0.0f, 0.0f, 0.0f}; return (Vector3){v.x / len, v.y / len, v.z / len}; }
#define qNormalize(V) _Generic((V), Vector2:qNormalize_v2, Vector3:qNormalize_v3 )(V)
// = = = = = DOT (DOT PRODUCT) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qDot_v2(Vector2 a, Vector2 b) { return a.x * b.x + a.y * b.y; }
static inline float qDot_v2i(Vector2Int a, Vector2Int b) { return a.x * b.x + a.y * b.y; }
static inline float qDot_v3(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline float qDot_v3i(Vector3Int a, Vector3Int b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
#define qDot(A, B) _Generic((A), Vector2:qDot_v2, Vector2Int:qDot_v2i, Vector3:qDot_v3, Vector3Int:qDot_v3i )(A, B)
// = = = = = CROSS (CROSS PRODUCT) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qCross_v2(Vector2 a, Vector2 b) { return a.x * b.y - a.y * b.x; }
static inline Vector3 qCross_v3(Vector3 a, Vector3 b) {	return (Vector3){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
#define qCross(A, B) _Generic((A), Vector2:qCross_v2, Vector3:qCross_v3 )(A, B)
// = = = = = DEGREES & RADIANS = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qDegToRad(float deg) { return deg * (PI / 180.0f); }
static inline float qRadToDeg(float rad) { return rad * (180.0f / PI); }
// = = = = = MIN = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qMin_i(int a, int b) { return a < b ? a : b; }
static inline float qMin_f(float a, float b) { return a < b ? a : b; }
#define qMin(A, B) _Generic((A), int:qMin_i, float:qMin_f)(A, B)
// = = = = = MAX = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qMax_i(int a, int b) { return a > b ? a : b; }
static inline float qMax_f(float a, float b) { return a > b ? a : b; }
#define qMax(A, B) _Generic((A), int:qMax_i, float:qMax_f)(A, B)
// = = = = = AABB (2D) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qAABB2D(Vector2 posA, Vector2 sizeA, Vector2 posB, Vector2 sizeB) {
	float minAx = posA.x - sizeA.x * 0.5f, maxAx = posA.x + sizeA.x * 0.5f, minAy = posA.y - sizeA.y * 0.5f, maxAy = posA.y + sizeA.y * 0.5f,
	minBx = posB.x - sizeB.x * 0.5f, maxBx = posB.x + sizeB.x * 0.5f, minBy = posB.y - sizeB.y * 0.5f, maxBy = posB.y + sizeB.y * 0.5f;
	return (minAx < maxBx && maxAx > minBx && minAy < maxBy && maxAy > minBy);
}

#endif
