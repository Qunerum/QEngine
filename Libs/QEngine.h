#ifndef QENGINE_MAIN_H
#define QENGINE_MAIN_H

#include <stddef.h>
#include "../Data/PROJECT.h"

#define QENGINE_VERSION_MAJOR 0
#define QENGINE_VERSION_MINOR 4
#define QENGINE_VERSION_PATCH 5

typedef enum { World, UI } qeDrawingMode;
typedef enum { Top_Left, Top, Top_Right, Left, Center, Right, Bottom_Left, Bottom, Bottom_Right } qeAlignMode;
// = = = = = TYPES = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
typedef long int64;
typedef unsigned long uint64;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef char int8;
typedef unsigned char uint8;

typedef uint32 uint;
typedef uint8 byte;
// = = = = = state = = = = = = = = = = = = = = = = = = = =
typedef uint8 state;
#define false 0
#define true 1
// = = = = = STRUCTURES = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// = = = = = Vector2 = = = = = = = = = = = = = = = = = = = =
typedef struct { float x, y; } Vector2;
#define _GET_VEC2_MACRO(_1, _2, NAME, ...) NAME
#define _Vec2_1(x) ((Vector2){ (x), 0.0f })
#define _Vec2_2(x, y) ((Vector2){ (x), (y) })
#define Vector2(...) _GET_VEC2_MACRO(__VA_ARGS__, _Vec2_2, _Vec2_1)(__VA_ARGS__)
#define Vec2(...) _GET_VEC2_MACRO(__VA_ARGS__, _Vec2_2, _Vec2_1)(__VA_ARGS__)
#define V2(...) _GET_VEC2_MACRO(__VA_ARGS__, _Vec2_2, _Vec2_1)(__VA_ARGS__)

#define Vector2_Zero V2(0, 0)
#define V2_Zero V2(0, 0)
#define Vector2_One V2(1, 1)
#define V2_One V2(1, 1)
#define Vector2_Up V2(0, 1)
#define V2_Up V2(0, 1)
#define Vector2_Down V2(0, -1)
#define V2_Down V2(0, -1)
#define Vector2_Left V2(-1, 0)
#define V2_Left V2(-1, 0)
#define Vector2_Right V2(1, 0)
#define V2_Right V2(1, 0)
// = = = = = Vector2Int = = = = = = = = = = = = = = = = = = = =
typedef struct { int x, y; } Vector2Int;
#define _GET_VEC2I_MACRO(_1, _2, NAME, ...) NAME
#define _Vec2I_1(x) ((Vector2Int){ (x), 0 })
#define _Vec2I_2(x, y) ((Vector2Int){ (x), (y) })
#define Vector2Int(...) _GET_VEC2I_MACRO(__VA_ARGS__, _Vec2I_2, _Vec2I_1)(__VA_ARGS__)
#define Vec2Int(...) _GET_VEC2I_MACRO(__VA_ARGS__, _Vec2I_2, _Vec2I_1)(__VA_ARGS__)
#define V2I(...) _GET_VEC2I_MACRO(__VA_ARGS__, _Vec2I_2, _Vec2I_1)(__VA_ARGS__)

#define Vector2Int_Zero V2I(0, 0)
#define V2I_Zero V2I(0, 0)
#define Vector2Int_One V2I(1, 1)
#define V2I_One V2I(1, 1)
#define Vector2Int_Up V2I(0, 1)
#define V2I_Up V2I(0, 1)
#define Vector2Int_Down V2I(0, -1)
#define V2I_Down V2I(0, -1)
#define Vector2Int_Left V2I(-1, 0)
#define V2I_Left V2I(-1, 0)
#define Vector2Int_Right V2I(1, 0)
#define V2I_Right V2I(1, 0)
// = = = = = Vector3 = = = = = = = = = = = = = = = = = = = =
typedef struct { float x, y, z; } Vector3;
#define _GET_VEC3_MACRO(_1, _2, _3, NAME, ...) NAME
#define _Vec3_1(x) ((Vector3){ (x), 0.0f, 0.0f })
#define _Vec3_2(x, y) ((Vector3){ (x), (y), 0.0f })
#define _Vec3_3(x, y, z) ((Vector3){ (x), (y), (z) })
#define Vector3(...) _GET_VEC3_MACRO(__VA_ARGS__, _Vec3_3, _Vec3_2, _Vec3_1)(__VA_ARGS__)
#define Vec3(...) _GET_VEC3_MACRO(__VA_ARGS__, _Vec3_3, _Vec3_2, _Vec3_1)(__VA_ARGS__)
#define V3(...) _GET_VEC3_MACRO(__VA_ARGS__, _Vec3_3, _Vec3_2, _Vec3_1)(__VA_ARGS__)

#define Vector3_Zero V3(0, 0, 0)
#define V3_Zero V3(0, 0, 0)
#define Vector3_One V3(1, 1, 1)
#define V3_One V3(1, 1, 1)
#define Vector3_Up V3(0, 1, 0)
#define V3_Up V3(0, 1, 0)
#define Vector3_Down V3(0, -1, 0)
#define V3_Down V3(0, -1, 0)
#define Vector3_Left V3(-1, 0, 0)
#define V3_Left V3(-1, 0, 0)
#define Vector3_Right V3(1, 0, 0)
#define V3_Right V3(1, 0, 0)
#define Vector3_Forward V3(0, 0, 1)
#define V3_Forward V3(0, 0, 1)
#define Vector3_Backward V3(0, 0, -1)
#define V3_Backward V3(0, 0, -1)
// = = = = = Vector3Int = = = = = = = = = = = = = = = = = = = =
typedef struct { int x, y, z; } Vector3Int;
#define _GET_VEC3I_MACRO(_1, _2, _3, NAME, ...) NAME
#define _Vec3I_1(x) ((Vector3Int){ (x), 0, 0 })
#define _Vec3I_2(x, y) ((Vector3Int){ (x), (y), 0 })
#define _Vec3I_3(x, y, z) ((Vector3Int){ (x), (y), (z) })
#define Vector3Int(...) _GET_VEC3I_MACRO(__VA_ARGS__, _Vec3I_3, _Vec3I_2, _Vec3I_1)(__VA_ARGS__)
#define Vec3Int(...) _GET_VEC3I_MACRO(__VA_ARGS__, _Vec3I_3, _Vec3I_2, _Vec3I_1)(__VA_ARGS__)
#define V3I(...) _GET_VEC3I_MACRO(__VA_ARGS__, _Vec3I_3, _Vec3I_2, _Vec3I_1)(__VA_ARGS__)

#define Vector3Int_Zero V3I(0, 0, 0)
#define V3I_Zero V3I(0, 0, 0)
#define Vector3Int_One V3I(1, 1, 1)
#define V3I_One V3I(1, 1, 1)
#define Vector3Int_Up V3I(0, 1, 0)
#define V3I_Up V3I(0, 1, 0)
#define Vector3Int_Down V3I(0, -1, 0)
#define V3I_Down V3I(0, -1, 0)
#define Vector3Int_Left V3I(-1, 0, 0)
#define V3I_Left V3I(-1, 0, 0)
#define Vector3Int_Right V3I(1, 0, 0)
#define V3I_Right V3I(1, 0, 0)
#define Vector3Int_Forward V3I(0, 0, 1)
#define V3I_Forward V3I(0, 0, 1)
#define Vector3Int_Backward V3I(0, 0, -1)
#define V3I_Backward V3I(0, 0, -1)
// = = = = = Transform = = = = = = = = = = = = = = = = = = = =
typedef struct { Vector3 position, rotation, scale; } Transform;
// = = = = = Camera = = = = = = = = = = = = = = = = = = = =
typedef struct {
	Vector3 position, rotation;
	float fov;
} Camera;
// = = = = = QObject = = = = = = = = = = = = = = = = = = = =
typedef struct {
	char name[MAX_NAME_LENGTH];
	Transform transform;
	state isActive;
} QObject;
// = = = = = COLOR = = = = = = = = = = = = = = = = = = = =
typedef struct { byte r, g, b, a; } Color;
#define _GET_COLOR_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define _Color_1(x) ((Color){ (x), (x), (x), 255 })
#define _Color_2(x, a) ((Color){ (x), (x), (x), (a) })
#define _Color_3(r, g, b) ((Color){ (r), (g), (b), 255 })
#define _Color_4(r, g, b, a) ((Color){ (r), (g), (b), (a) })
#define Color(...) _GET_COLOR_MACRO(__VA_ARGS__, _Color_4, _Color_3, _Color_2, _Color_1)(__VA_ARGS__)
#define Clr(...) _GET_COLOR_MACRO(__VA_ARGS__, _Color_4, _Color_3, _Color_2, _Color_1)(__VA_ARGS__)
// = = = = = COLORS = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
#define Color_Transparent   Clr(0, 0)
#define Color_Black         Clr(0)
#define Color_White         Clr(255)
// = = = = = LIGHT = = = = = = = = = = = = = = = = = = = =
#define Color_Light_Gray    Clr(192, 192, 192)
#define Color_Light_Red     Clr(255, 64, 64)
#define Color_Light_Green   Clr(128, 255, 128)
#define Color_Light_Yellow  Clr(255, 255, 128)
#define Color_Light_Orange  Clr(255, 192, 64)
#define Color_Light_Blue    Clr(64, 64, 255)
#define Color_Light_Magenta Clr(255, 64, 255)
#define Color_Light_Cyan    Clr(128, 255, 255)
// = = = = = NORMAL = = = = = = = = = = = = = = = = = = = =
#define Color_Gray          Clr(128, 128, 128)
#define Color_Red           Clr(255, 0, 0)
#define Color_Green         Clr(0, 255, 0)
#define Color_Yellow        Clr(255, 255, 0)
#define Color_Orange        Clr(255, 128, 0)
#define Color_Blue          Clr(0, 0, 255)
#define Color_Magenta       Clr(255, 0, 255)
#define Color_Cyan          Clr(0, 255, 255)
// = = = = = DARK = = = = = = = = = = = = = = = = = = = =
#define Color_Dark_Gray     Clr(64, 64, 64)
#define Color_Dark_Red      Clr(128, 0, 0)
#define Color_Dark_Green    Clr(0, 128, 0)
#define Color_Dark_Yellow   Clr(128, 128, 0)
#define Color_Dark_Orange   Clr(128, 64, 0)
#define Color_Dark_Blue     Clr(0, 0, 128)
#define Color_Dark_Magenta  Clr(128, 0, 128)
#define Color_Dark_Cyan     Clr(0, 128, 128)

// = = = = = FUNCTIONS = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void print(const char* format, ...);
state formatText(char* to, const uint length, const char* format, ...);

int initEngineProject(void (*initFunc)(), void (*updateFunc)());

void setDrawingMode(const qeDrawingMode mode);

uint getWidth();
uint getHeight();

Camera getCamera();
void setCamera(const Camera camera);
void setCameraPos(const Vector3 position);
void setCameraRot(const Vector3 rotation);
void setCameraScale(const Vector3 scale);

void drawTriangle(const Vector3 posA, const Vector3 posB, const Vector3 posC, const Color color);
void drawRect(const Vector3 position, const Vector3 rotation, const Vector2 size, const qeAlignMode align, const Color color);
void drawCircle(const Vector3 position, const Vector3 rotation, const uint segments, const float radius, const qeAlignMode align, const Color color);
void drawText(const char* text, const Vector3 position, const Vector3 rotation, const float fontSize, const qeAlignMode align, const Color color);

state drawButton(const Vector3 position, const Vector3 rotation, const Vector2 size, const qeAlignMode align, const Color clrBase, const Color clrHover, const Color clrPress);

void drawBox(const Vector3 position, const Vector3 rotation, const Vector3 size, const Color color);
void drawSphere(const Vector3 position, const Vector3 rotation, const uint rings, const uint sectors, const float radius, const Color color);

#ifdef QEngine_Audio
void convertAudio(const char* qsr_path, const char* qs_path);
uint loadAudio(const char* path);
void playAudio(const uint audioID, const uint8 volume, const float speed);
#endif

#ifdef QEngine_Input
#define MAX_INPUT 1024

void clearInput();
void enableInput();
void disableInput();
state getInputState();
void setMaxInput(uint max);
void getInput(char* buffor, const uint length);
state getKeyState(const uint keyCode);
state onKeyDown(const uint keyCode);
char getPressedKey();
state getMouseButton(const uint mouseKey);
state onMouseDown(const uint mouseKey);
Vector2 getCursorPosition();

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
#define KEY_CAPSLOCK   280
#endif

#ifdef QEngine_Math
#define PI 3.14159265358979323846f
// = = = = = LERP = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
float qLerp_f(const float A, const float B, const float t);
Vector2 qLerp_v2(const Vector2 A, const Vector2 B, const float t);
Vector3 qLerp_v3(const Vector3 A, const Vector3 B, const float t);
#define qLerp(A, B, t) _Generic((A), float:qLerp_f, Vector2:qLerp_v2, Vector3:qLerp_v3 )(A, B, t)
// = = = = = CLAMP = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qClamp_i(const int v, const int min, const int max) { return v > max ? max : v < min ? min : v; }
static inline float qClamp_f(const float v, const float min, const float max) { return v > max ? max : v < min ? min : v; }
static inline Vector2 qClamp_v2(const Vector2 v, const Vector2 min, const Vector2 max) {
	return (Vector2){
		qClamp_f(v.x, min.x, max.x),
		qClamp_f(v.y, min.y, max.y)};
}
static inline Vector2Int qClamp_v2i(const Vector2Int v, const Vector2Int min, const Vector2Int max) {
	return (Vector2Int){
		qClamp_i(v.x, min.x, max.x),
		qClamp_i(v.y, min.y, max.y)};
}
static inline Vector3 qClamp_v3(const Vector3 v, const Vector3 min, const Vector3 max) {
	return (Vector3){
		qClamp_f(v.x, min.x, max.x),
		qClamp_f(v.y, min.y, max.y),
		qClamp_f(v.z, min.z, max.z)};
}
static inline Vector3Int qClamp_v3i(const Vector3Int v, const Vector3Int min, const Vector3Int max) {
	return (Vector3Int){
		qClamp_i(v.x, min.x, max.x),
		qClamp_i(v.y, min.y, max.y),
		qClamp_i(v.z, min.z, max.z)};
}
#define qClamp(Value, Min, Max) _Generic((Value), int:qClamp_i, float:qClamp_f, Vector2:qClamp_v2, Vector2Int:qClamp_v2i, Vector3:qClamp_v3, Vector3Int:qClamp_v3i )(Value, Min, Max)
// = = = = = MAP = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qMap_f(const float v, const float oldMin, const float oldMax, const float newMin, const float newMax) {
	if (oldMax - oldMin == 0.0f) return newMin;
	return qLerp_f(newMin, newMax, (v - oldMin) / (oldMax - oldMin));
}
static inline Vector2 qMap_v2(const Vector2 v, const Vector2 oldMin, const Vector2 oldMax, const Vector2 newMin, const Vector2 newMax) {
	return (Vector2){
		qMap_f(v.x, oldMin.x, oldMax.x, newMin.x, newMax.x),
		qMap_f(v.y, oldMin.y, oldMax.y, newMin.y, newMax.y)};
}
static inline Vector3 qMap_v3(const Vector3 v, const Vector3 oldMin, const Vector3 oldMax, const Vector3 newMin, const Vector3 newMax) {
	return (Vector3){
		qMap_f(v.x, oldMin.x, oldMax.x, newMin.x, newMax.x),
		qMap_f(v.y, oldMin.y, oldMax.y, newMin.y, newMax.y),
		qMap_f(v.z, oldMin.z, oldMax.z, newMin.z, newMax.z)};
}
#define qMap(Value, oldMin, oldMax, newMin, newMax) _Generic((Value), float:qMap_f, Vector2:qMap_v2, Vector3:qMap_v3 )(Value, oldMin, oldMax, newMin, newMax)
// = = = = = ABS (ABSOLUTE) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qAbs_i(const int v) { return v < 0 ? -v : v; }
static inline float qAbs_f(const float v) { return v < 0.0f ? -v : v; }
static inline Vector2 qAbs_v2(const Vector2 v) {
	return (Vector2){
		qAbs_f(v.x),
		qAbs_f(v.y)};
}
static inline Vector2Int qAbs_v2i(const Vector2Int v) {
	return (Vector2Int){
		qAbs_i(v.x),
		qAbs_i(v.y)};
}
static inline Vector3 qAbs_v3(const Vector3 v) {
	return (Vector3){
		qAbs_f(v.x),
		qAbs_f(v.y),
		qAbs_f(v.z)};
}
static inline Vector3Int qAbs_v3i(const Vector3Int v) {
	return (Vector3Int){
		qAbs_i(v.x),
		qAbs_i(v.y),
		qAbs_i(v.z)};
}
#define qAbs(V) _Generic((V), int:qAbs_i, float:qAbs_f, Vector2:qAbs_v2, Vector2Int:qAbs_v2i, Vector3:qAbs_v3, Vector3Int:qAbs_v3i )(V)
// = = = = = SIGN = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qSign_i(const int v) { return (v > 0) - (v < 0); }
static inline float qSign_f(const float v) { return (float)((v > 0.0f) - (v < 0.0f)); }
static inline Vector2 qSign_v2(const Vector2 v) {
	return (Vector2){
		qSign_f(v.x),
		qSign_f(v.y)};
}
static inline Vector2Int qSign_v2i(const Vector2Int v) {
	return (Vector2Int){
		qSign_i(v.x),
		qSign_i(v.y)};
}
static inline Vector3 qSign_v3(const Vector3 v) {
	return (Vector3){
		qSign_f(v.x),
		qSign_f(v.y),
		qSign_f(v.z)};
}
static inline Vector3Int qSign_v3i(const Vector3Int v) {
	return (Vector3Int){
		qSign_i(v.x),
		qSign_i(v.y),
		qSign_i(v.z)};
}
#define qSign(V) _Generic((V), int:qSign_i, float:qSign_f, Vector2:qSign_v2, Vector2Int:qSign_v2i, Vector3:qSign_v3, Vector3Int:qSign_v3i )(V)
// = = = = = ROUNDING = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
int qFloor(const float v);
int qCeil(const float v);
static inline int qRound(const float v) { return v < 0.0f ? (int)(v - 0.5f) : (int)(v + 0.5f); }
// = = = = = POW (POWER) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qPow(const float base, const int exp) {
	if (exp == 0) return 1.0f;
	if (base == 0.0f) return 0.0f;
	float v = base;
	int absExp = qAbs_i(exp);
	for (int i = 1; i < absExp; i++) v *= base;
	if (exp < 0) v = 1.0f / v;
	return v;
}
// = = = = = SQRT (SQUARE ROOT) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
float qSqrt(const float number);
// = = = = = DIST (DISTANCE) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qDist_v2(const Vector2 a, const Vector2 b) { return qSqrt(qPow(b.x - a.x, 2) + qPow(b.y - a.y, 2)); }
static inline float qDist_v3(const Vector3 a, const Vector3 b) { return qSqrt(qPow(b.x - a.x, 2) + qPow(b.y - a.y, 2) + qPow(b.z - a.z, 2)); }
#define qDist(A, B) _Generic((A), Vector2:qDist_v2, Vector3:qDist_v3 )(A, B)
// = = = = = LENGTH = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qLength_v2(const Vector2 v) { return qSqrt(v.x * v.x + v.y * v.y); }
static inline float qLength_v3(const Vector3 v) { return qSqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
#define qLength(V) _Generic((V), Vector2:qLength_v2, Vector3:qLength_v3 )(V)
// = = = = = NORMALIZE = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
Vector2 qNormalize_v2(const Vector2 v);
Vector3 qNormalize_v3(const Vector3 v);
#define qNormalize(V) _Generic((V), Vector2:qNormalize_v2, Vector3:qNormalize_v3 )(V)
// = = = = = DOT (DOT PRODUCT) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qDot_v2(const Vector2 a, const Vector2 b) { return a.x * b.x + a.y * b.y; }
static inline float qDot_v2i(const Vector2Int a, const Vector2Int b) { return a.x * b.x + a.y * b.y; }
static inline float qDot_v3(const Vector3 a, const Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline float qDot_v3i(const Vector3Int a, const Vector3Int b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
#define qDot(A, B) _Generic((A), Vector2:qDot_v2, Vector2Int:qDot_v2i, Vector3:qDot_v3, Vector3Int:qDot_v3i )(A, B)
// = = = = = CROSS (CROSS PRODUCT) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qCross_v2(const Vector2 a, const Vector2 b) { return a.x * b.y - a.y * b.x; }
static inline Vector3 qCross_v3(const Vector3 a, const Vector3 b) { return (Vector3){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
#define qCross(A, B) _Generic((A), Vector2:qCross_v2, Vector3:qCross_v3 )(A, B)
// = = = = = DEGREES & RADIANS = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline float qDegToRad(const float deg) { return deg * (PI / 180.0f); }
static inline float qRadToDeg(const float rad) { return rad * (180.0f / PI); }
// = = = = = MIN = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qMin_i(const int a, const int b) { return a < b ? a : b; }
static inline float qMin_f(const float a, const float b) { return a < b ? a : b; }
#define qMin(A, B) _Generic((A), int:qMin_i, float:qMin_f)(A, B)
// = = = = = MAX = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline int qMax_i(const int a, const int b) { return a > b ? a : b; }
static inline float qMax_f(const float a, const float b) { return a > b ? a : b; }
#define qMax(A, B) _Generic((A), int:qMax_i, float:qMax_f)(A, B)
// = = = = = AABB (2D) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline state qAABB2D(const Vector2 posA, const Vector2 sizeA, const Vector2 posB, const Vector2 sizeB) {
	float minAx = posA.x - sizeA.x * 0.5f, maxAx = posA.x + sizeA.x * 0.5f,
	minAy = posA.y - sizeA.y * 0.5f, maxAy = posA.y + sizeA.y * 0.5f,
	minBx = posB.x - sizeB.x * 0.5f, maxBx = posB.x + sizeB.x * 0.5f,
	minBy = posB.y - sizeB.y * 0.5f, maxBy = posB.y + sizeB.y * 0.5f;
	return (minAx < maxBx && maxAx > minBx && minAy < maxBy && maxAy > minBy);
}
// = = = = = Math on Vectors = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static inline Vector2 Vector2_Add(const Vector2 A, const Vector2 B) { return (Vector2){A.x + B.x, A.y + B.y}; }
static inline Vector2 Vector2_Sub(const Vector2 A, const Vector2 B) { return (Vector2){A.x - B.x, A.y - B.y}; }
static inline Vector2 Vector2_Mul(const Vector2 A, const float B) { return (Vector2){A.x * B, A.y * B}; }
static inline Vector2 Vector2_Div(const Vector2 A, const float B) { if(B == 0.0f) { print("Cannot divide by zero!\n"); return Vector2_Zero; } return (Vector2){A.x / B, A.y / B}; }
static inline Vector2Int Vector2Int_Add(const Vector2Int A, const Vector2Int B) { return (Vector2Int){A.x + B.x, A.y + B.y}; }
static inline Vector2Int Vector2Int_Sub(const Vector2Int A, const Vector2Int B) { return (Vector2Int){A.x - B.x, A.y - B.y}; }

static inline Vector3 Vector3_Add(const Vector3 A, const Vector3 B) { return (Vector3){A.x + B.x, A.y + B.y, A.z + B.z}; }
static inline Vector3 Vector3_Sub(const Vector3 A, const Vector3 B) { return (Vector3){A.x - B.x, A.y - B.y, A.z - B.z}; }
static inline Vector3 Vector3_Mul(const Vector3 A, const float B) { return (Vector3){A.x * B, A.y * B, A.z * B}; }
static inline Vector3 Vector3_Div(const Vector3 A, const float B) { if(B == 0.0f) { print("Cannot divide by zero!\n"); return Vector3_Zero; } return (Vector3){A.x / B, A.y / B, A.z / B}; }
static inline Vector3Int Vector3Int_Add(const Vector3Int A, const Vector3Int B) { return (Vector3Int){A.x + B.x, A.y + B.y, A.z + B.z}; }
static inline Vector3Int Vector3Int_Sub(const Vector3Int A, const Vector3Int B) { return (Vector3Int){A.x - B.x, A.y - B.y, A.z - B.z}; }
#endif

#ifdef QEngine_Memory
void* qMalloc(const uint size);
void *qRealloc(void *ptr, const uint size);
void qFree(void* ptr);
#endif

#ifdef QEngine_IO
char* qReadFileText(const char* filename);
state qWriteFileText(const char* filename, const char* text);
state qFileExists(const char* filename);
#endif

#ifdef QEngine_Text
static inline uint qLenStr(const char* text) {
	if (!text) return 0;
	uint i = 0;
	while(text[i] != '\0') i++;
	return i;
}
static inline void qCopy(char* dest, uint destLength, const char* src) {
	if (!dest || destLength == 0 || !src) return;
	uint i = 0;
	while(src[i] && i < destLength - 1) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}
static inline state qIs(const char* s1, const char* s2, const uint n) {
	if (n == 0) return true;
	if (!s1 || !s2) return s1 == s2;
	for (uint i = 0; i < n; i++) {
		if (s1[i] != s2[i]) return false;
		if (s1[i] == '\0') return true;
	}
	return true;
}
static inline void qAdd(char* dest, const uint destLength, const char* src) {
	if (!dest || destLength == 0 || !src) return;
	uint i = qLenStr(dest), j = 0;
	if (i >= destLength - 1) return;
	while (src[j] && i < destLength - 1) {
		dest[i] = src[j];
		i++;
		j++;
	}
	dest[i] = '\0';
}
static inline void qReverse(char* text) {
	if (!text) return;
	uint l = qLenStr(text);
	if (l <= 1) return;
	uint a = 0, b = l - 1;
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
