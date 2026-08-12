#ifndef QENGINE_MAIN_H
#define QENGINE_MAIN_H

#include <stdint.h>
#include <stddef.h>
#include "../Data/PROJECT.h"

#define QENGINE_VERSION_MAJOR 0
#define QENGINE_VERSION_MINOR 1
#define QENGINE_VERSION_PATCH 3

typedef enum { World, UI } qeDrawingMode;
// = = = = = TYPES = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
typedef uint8_t byte;
typedef int state;
#define false 0
#define true 1
// = = = = = STRUCTURES = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// = = = = = VECTORS = = = = = = = = = = = = = = = = = = = =
typedef struct { float x, y; } Vector2;
#define Vector2_Zero        (Vector2){0, 0}
#define Vector2_One         (Vector2){1, 1}
#define Vector2_Up          (Vector2){0, 1}
#define Vector2_Down        (Vector2){0, -1}
#define Vector2_Left        (Vector2){-1, 0}
#define Vector2_Right       (Vector2){1, 0}
typedef struct { int x, y; } Vector2Int;
#define Vector2Int_Zero     (Vector2Int){0, 0}
#define Vector2Int_One      (Vector2Int){1, 1}
#define Vector2Int_Up       (Vector2Int){0, 1}
#define Vector2Int_Down     (Vector2Int){0, -1}
#define Vector2Int_Left     (Vector2Int){-1, 0}
#define Vector2Int_Right    (Vector2Int){1, 0}
typedef struct { float x, y, z; } Vector3;
#define Vector3_Zero        (Vector3){0, 0, 0}
#define Vector3_One         (Vector3){1, 1, 1}
#define Vector3_Up          (Vector3){0, 1, 0}
#define Vector3_Down        (Vector3){0, -1, 0}
#define Vector3_Left        (Vector3){-1, 0, 0}
#define Vector3_Right       (Vector3){1, 0, 0}
#define Vector3_Forward     (Vector3){0, 0, 1}
#define Vector3_Backward    (Vector3){0, 0, -1}
typedef struct { int x, y, z; } Vector3Int;
#define Vector3Int_Zero     (Vector3Int){0, 0, 0}
#define Vector3Int_One      (Vector3Int){1, 1, 1}
#define Vector3Int_Up       (Vector3Int){0, 1, 0}
#define Vector3Int_Down     (Vector3Int){0, -1, 0}
#define Vector3Int_Left     (Vector3Int){-1, 0, 0}
#define Vector3Int_Right    (Vector3Int){1, 0, 0}
#define Vector3Int_Forward  (Vector3Int){0, 0, 1}
#define Vector3Int_Backward (Vector3Int){0, 0, -1}
// = = = = = END VECTORS = = = = = = = = = = = = = = = = = = = =
typedef struct { Vector3 position, rotation, scale; } Transform;
typedef struct {
	Vector3 position, rotation;
	float fov;
} Camera;
typedef struct {
	char name[MAX_NAME_LENGTH];
	Transform transform;
	state isActive;
} QObject;
// = = = = = COLORS = = = = = = = = = = = = = = = = = = = =
typedef struct { byte r, g, b, a; } Color;
#define Color_Transparent   (Color){  0,   0,   0,   0}
#define Color_Black         (Color){  0,   0,   0, 255}
#define Color_White         (Color){255, 255, 255, 255}
// ===== Light ========================================
#define Color_Light_Gray    (Color){192, 192, 192, 255}
#define Color_Light_Red     (Color){255,  64,  64, 255}
#define Color_Light_Green   (Color){128, 255, 128, 255}
#define Color_Light_Yellow  (Color){255, 255, 128, 255}
#define Color_Light_Orange  (Color){255, 192,  64, 255}
#define Color_Light_Blue    (Color){ 64,  64, 255, 255}
#define Color_Light_Magenta (Color){255,  64, 255, 255}
#define Color_Light_Cyan    (Color){128, 255, 255, 255}
// ===== Normal =======================================
#define Color_Gray          (Color){128, 128, 128, 255}
#define Color_Red           (Color){255,   0,   0, 255}
#define Color_Green         (Color){  0, 255,   0, 255}
#define Color_Yellow        (Color){255, 255,   0, 255}
#define Color_Orange        (Color){255, 128,   0, 255}
#define Color_Blue          (Color){  0,   0, 255, 255}
#define Color_Magenta       (Color){255,   0, 255, 255}
#define Color_Cyan          (Color){  0, 255, 255, 255}
// ===== Dark =========================================
#define Color_Dark_Gray     (Color){ 64,  64,  64, 255}
#define Color_Dark_Red      (Color){128,   0,   0, 255}
#define Color_Dark_Green    (Color){  0, 128,   0, 255}
#define Color_Dark_Yellow   (Color){128, 128,   0, 255}
#define Color_Dark_Orange   (Color){128,  64,   0, 255}
#define Color_Dark_Blue     (Color){  0,   0, 128, 255}
#define Color_Dark_Magenta  (Color){128,   0, 128, 255}
#define Color_Dark_Cyan     (Color){  0, 128, 128, 255}
// = = = = = END COLORS = = = = = = = = = = = = = = = = = = = =

// = = = = = FUNCTIONS = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void print(const char* format, ...);
int formatText(char* to, int length, const char* format, ...);

int initEngineProject(void (*initFunc)(), void (*updateFunc)());

void setDrawingMode(qeDrawingMode mode);

Camera getCamera();
void setCamera(Camera camera);
void setCameraPos(Vector3 position);
void setCameraRot(Vector3 rotation);
void setCameraScale(Vector3 scale);

#ifdef QEngine_Input
int getKeyState(int keyCode);
int onKeyDown(int keyCode);
int getMouseButton(int mouseKey);
int onMouseDown(int mouseKey);
Vector2 getCursorPosition();

// = = = = = KEYS = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
#define LMB             0
#define RMB             1
#define KEY_A          65
#define KEY_B          66
#define KEY_C          67
#define KEY_D          68
#define KEY_E          69
#define KEY_F          70
#define KEY_G          71
#define KEY_H          72
#define KEY_I          73
#define KEY_J          74
#define KEY_K          75
#define KEY_L          76
#define KEY_M          77
#define KEY_N          78
#define KEY_O          79
#define KEY_P          80
#define KEY_Q          81
#define KEY_R          82
#define KEY_S          83
#define KEY_T          84
#define KEY_U          85
#define KEY_V          86
#define KEY_W          87
#define KEY_X          88
#define KEY_Y          89
#define KEY_Z          90
#define KEY_SPACE      32
#define KEY_ESCAPE     256
#define KEY_ENTER      257
#define KEY_BACKSPACE  259
#define KEY_LSHIFT     340
#define KEY_LCTRL      341
#define KEY_RIGHT      262
#define KEY_LEFT       263
#define KEY_DOWN       264
#define KEY_UP         265
#endif

#ifdef QEngine_Math
#define PI 3.14159265358979323846f
// = = = = = LERP = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
float qLerp_f(float A, float B, float t);
Vector2 qLerp_v2(Vector2 A, Vector2 B, float t);
Vector3 qLerp_v3(Vector3 A, Vector3 B, float t);
#define qLerp(A, B, t) _Generic((A), float:qLerp_f, Vector2:qLerp_v2, Vector3:qLerp_v3 )(A, B, t)
// = = = = = CLAMP = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qClamp_i(int v, int min, int max) { return v > max ? max : v < min ? min : v; }
static inline float qClamp_f(float v, float min, float max) { return v > max ? max : v < min ? min : v; }
static inline Vector2 qClamp_v2(Vector2 v, Vector2 min, Vector2 max) { return (Vector2){qClamp_f(v.x, min.x, max.x), qClamp_f(v.y, min.y, max.y)}; }
static inline Vector2Int qClamp_v2i(Vector2Int v, Vector2Int min, Vector2Int max) { return (Vector2Int){qClamp_i(v.x, min.x, max.x), qClamp_i(v.y, min.y, max.y)}; }
static inline Vector3 qClamp_v3(Vector3 v, Vector3 min, Vector3 max) { return (Vector3){qClamp_f(v.x, min.x, max.x), qClamp_f(v.y, min.y, max.y), qClamp_f(v.z, min.z, max.z)}; }
static inline Vector3Int qClamp_v3i(Vector3Int v, Vector3Int min, Vector3Int max) { return (Vector3Int){
	qClamp_i(v.x, min.x, max.x),
	qClamp_i(v.y, min.y, max.y),
	qClamp_i(v.z, min.z, max.z)};
}
#define qClamp(Value, Min, Max) _Generic((Value), int:qClamp_i, float:qClamp_f, Vector2:qClamp_v2, Vector2Int:qClamp_v2i, Vector3:qClamp_v3, Vector3Int:qClamp_v3i )(Value, Min, Max)
// = = = = = MAP = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qMap_f(float v, float oldMin, float oldMax, float newMin, float newMax) {
	if (oldMax - oldMin == 0.0f) return newMin;
	return qLerp_f(newMin, newMax, (v - oldMin) / (oldMax - oldMin));
}
static inline Vector2 qMap_v2(Vector2 v, Vector2 oldMin, Vector2 oldMax, Vector2 newMin, Vector2 newMax) { return (Vector2){
	qMap_f(v.x, oldMin.x, oldMax.x, newMin.x, newMax.x),
	qMap_f(v.y, oldMin.y, oldMax.y, newMin.y, newMax.y)};
}
static inline Vector3 qMap_v3(Vector3 v, Vector3 oldMin, Vector3 oldMax, Vector3 newMin, Vector3 newMax) { return (Vector3){
	qMap_f(v.x, oldMin.x, oldMax.x, newMin.x, newMax.x),
	qMap_f(v.y, oldMin.y, oldMax.y, newMin.y, newMax.y),
	qMap_f(v.z, oldMin.z, oldMax.z, newMin.z, newMax.z)};
}
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
static inline float qPow(float base, int exp) {
	if (exp == 0) return 1;
	float v = base;
	int absExp = qAbs_i(exp);
	for (int i = 1; i < absExp; i++) v *= base;
	if (exp < 0) v = 1.0f / v;
	return v;
}
// = = = = = SQRT (SQUARE ROOT) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
float qSqrt(float number);
// = = = = = DIST (DISTANCE) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qDist_v2(Vector2 a, Vector2 b) { return qSqrt(qPow(b.x - a.x, 2) + qPow(b.y - a.y, 2)); }
static inline float qDist_v3(Vector3 a, Vector3 b) { return qSqrt(qPow(b.x - a.x, 2) + qPow(b.y - a.y, 2) + qPow(b.z - a.z, 2)); }
#define qDist(A, B) _Generic((A), Vector2:qDist_v2, Vector3:qDist_v3 )(A, B)
// = = = = = LENGTH = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qLength_v2(Vector2 v) { return qSqrt(v.x * v.x + v.y * v.y); }
static inline float qLength_v3(Vector3 v) { return qSqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
#define qLength(V) _Generic((V), Vector2:qLength_v2, Vector3:qLength_v3 )(V)
// = = = = = NORMALIZE = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
Vector2 qNormalize_v2(Vector2 v);
Vector3 qNormalize_v3(Vector3 v);

#define qNormalize(V) _Generic((V), Vector2:qNormalize_v2, Vector3:qNormalize_v3 )(V)
// = = = = = DOT (DOT PRODUCT) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qDot_v2(Vector2 a, Vector2 b) { return a.x * b.x + a.y * b.y; }
static inline float qDot_v2i(Vector2Int a, Vector2Int b) { return a.x * b.x + a.y * b.y; }
static inline float qDot_v3(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline float qDot_v3i(Vector3Int a, Vector3Int b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
#define qDot(A, B) _Generic((A), Vector2:qDot_v2, Vector2Int:qDot_v2i, Vector3:qDot_v3, Vector3Int:qDot_v3i )(A, B)
// = = = = = CROSS (CROSS PRODUCT) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qCross_v2(Vector2 a, Vector2 b) { return a.x * b.y - a.y * b.x; }
static inline Vector3 qCross_v3(Vector3 a, Vector3 b) { return (Vector3){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
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
	float minAx = posA.x - sizeA.x * 0.5f, maxAx = posA.x + sizeA.x * 0.5f,
	minAy = posA.y - sizeA.y * 0.5f, maxAy = posA.y + sizeA.y * 0.5f,
	minBx = posB.x - sizeB.x * 0.5f, maxBx = posB.x + sizeB.x * 0.5f,
	minBy = posB.y - sizeB.y * 0.5f, maxBy = posB.y + sizeB.y * 0.5f;
	return (minAx < maxBx && maxAx > minBx && minAy < maxBy && maxAy > minBy);
}
// = = = = = Math on Vectors = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline Vector2 Vector2_Add(Vector2 A, Vector2 B) { return (Vector2){A.x + B.x, A.y + B.y}; }
static inline Vector2 Vector2_Sub(Vector2 A, Vector2 B) { return (Vector2){A.x - B.x, A.y - B.y}; }
static inline Vector2 Vector2_Mul(Vector2 A, float B) { return (Vector2){A.x * B, A.y * B}; }
static inline Vector2 Vector2_Div(Vector2 A, float B) { if(B == 0.0f) { print("Cannot divide by zero!\n"); return Vector2_Zero; } return (Vector2){A.x / B, A.y / B}; }
static inline Vector2Int Vector2Int_Add(Vector2Int A, Vector2Int B) { return (Vector2Int){A.x + B.x, A.y + B.y}; }
static inline Vector2Int Vector2Int_Sub(Vector2Int A, Vector2Int B) { return (Vector2Int){A.x - B.x, A.y - B.y}; }

static inline Vector3 Vector3_Add(Vector3 A, Vector3 B) { return (Vector3){A.x + B.x, A.y + B.y, A.z + B.z}; }
static inline Vector3 Vector3_Sub(Vector3 A, Vector3 B) { return (Vector3){A.x - B.x, A.y - B.y, A.z - B.z}; }
static inline Vector3 Vector3_Mul(Vector3 A, float B) { return (Vector3){A.x * B, A.y * B, A.z * B}; }
static inline Vector3 Vector3_Div(Vector3 A, float B) { if(B == 0.0f) { print("Cannot divide by zero!\n"); return Vector3_Zero; } return (Vector3){A.x / B, A.y / B, A.z / B}; }
static inline Vector3Int Vector3Int_Add(Vector3Int A, Vector3Int B) { return (Vector3Int){A.x + B.x, A.y + B.y, A.z + B.z}; }
static inline Vector3Int Vector3Int_Sub(Vector3Int A, Vector3Int B) { return (Vector3Int){A.x - B.x, A.y - B.y, A.z - B.z}; }

#endif

#ifdef QEngine_Memory
void* qMalloc(size_t size);
void qFree(void* ptr);
#endif

#ifdef QEngine_IO
char* qReadFileText(const char* filename);
int qWriteFileText(const char* filename, const char* text);
int qFileExists(const char* filename);
#endif

#ifdef QEngine_Text
static inline int qLenStr(char* text) {
	int i = 0;
	while(text[i] != '\0') i++;
	return i;
}
static inline void qCopy(char* dest, char* src) {
	int i = 0;
	while(src[i]) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}
static inline int qIs(char* s1, char* s2) {
	int i = 0;
	while(s1[i] && s2[i]) {
		if (s1[i] != s2[i]) return 0;
		i++;
	}
	return s1[i] == s2[i];
}
static inline void qAdd(char* dest, char* src) {
	int i = qLenStr(dest), j = 0;
	while (src[j]) {
		dest[i] = src[j];
		i++;
		j++;
	}
	dest[i] = '\0';
}
static inline void qReverse(char* text) {
	int l = qLenStr(text), a = 0, b = l - 1;
	while (a < b) {
		char t = text[a];
		text[a] = text[b];
		text[b] = t;
		a++;
		b--;
	}
}
#endif

#endif
