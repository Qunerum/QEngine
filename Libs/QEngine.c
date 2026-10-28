#define QGPU_UI
#define QGPU_SHAPES

#define uint quint
#define Vector2 QV2
#define Vector3 QV3
#include "qgpu.h"
#undef uint
#undef Vector2
#undef Vector3

#define QEngine_Input
#define QEngine_Math
#define QEngine_Memory
#define QEngine_IO
#define QEngine_Text
#include "QEngine.h"

#include "qsound.h"
#include "../Data/PROJECT.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <dirent.h>
// = = = = = MEMORY = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void *qMalloc(const uint size) { return malloc(size); }
void *qRealloc(void *ptr, const uint size) { return realloc(ptr, size); }
void qFree(void *ptr) { free(ptr); }
// = = = = = IO = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
char* qReadFileText(const char* filename) {
	if (!filename) return NULL;
	FILE* file = fopen(filename, "rb");
	if (!file) return NULL;
	fseek(file, 0, SEEK_END);
	const long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* buffer = (char*)qMalloc(size + 1);
	if (!buffer) {
		fclose(file);
		return NULL;
	}
	fread(buffer, 1, size, file);
	buffer[size] = '\0';
	fclose(file);
	return buffer;
}
state qWriteFileText(const char* filename, const char* text) {
	if (!filename || !text) return false;
	FILE* file = fopen(filename, "w");
	if (!file) return false;
	const int result = fputs(text, file);
	fclose(file);
	return result != EOF;
}
state qFileExists(const char* filename) {
	if (!filename) return false;
	FILE* file = fopen(filename, "r");
	if (file) {
		fclose(file);
		return true;
	}
	return false;
}
state qPathExists(const char* path) {
	if (!path) return false;
	struct stat st;
	if (stat(path, &st) == 0) return true;
	return false;
}
static char buffer[8192];
state qCopyFile(const char* srcPath, const char* dstPath) {
	if (!srcPath || !dstPath) return false;
	FILE* src = fopen(srcPath, "rb");
	if (!src) return false;
	FILE* dst = fopen(dstPath, "wb");
	if (!dst) {
		fclose(src);
		return false;
	}
	size_t bytesRead;
	while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) fwrite(buffer, 1, bytesRead, dst);
	fclose(src);
	fclose(dst);
	return true;
}
// = = = = = ENGINE = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
extern void qgVprintc(const int color, const char* format, va_list args);
void print(const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(244, format, args);
	va_end(args);
}
state formatText(char* to, const uint length, const char* format, ...) {
	va_list args;
	va_start(args, format);
	const state written = vsnprintf(to, length, format, args);
	va_end(args);
	return written;
}

static Scene actualScene;
static Vector2 getPos(const Vector2 pos, const qeAlignMode align) {
	Vector2 v = pos;
	const float w = qgGetWidth() * 0.5f, h = qgGetHeight() * 0.5f;
	switch (align) {
		case Top_Left: v.y += h; v.x -= w; return v;
		case Top: v.y += h; return v;
		case Top_Right: v.y += h; v.x += w; return v;
		case Left: v.x -= w; return v;
		case Center: return v;
		case Right: v.x += w; return v;
		case Bottom_Left: v.y -= h; v.x -= w; return v;
		case Bottom: v.y -= h; return v;
		case Bottom_Right: v.y -= h; v.x += w; return v;
	}
	return v;
}
static void qeInit() {
	print("Application was made in QEngine v%i.%i.%i\n", QENGINE_VERSION_MAJOR, QENGINE_VERSION_MINOR, QENGINE_VERSION_PATCH);
	if (actualScene.init) actualScene.init();
}
static state isCaps = false, inputOn = false;
static char qinput[MAX_INPUT + 1];
static uint inputLen = 0, userMax = MAX_INPUT;
static Vector2 prevMP = V2_Zero, deltaMP = V2_Zero;

static void qeUpdate() {
	if (onKeyDown(KEY_CAPSLOCK)) isCaps = !isCaps;
	const char c = getPressedKey();
	if (c && inputOn) {
		if (c == '\b' && inputLen > 0) {
			inputLen--;
			qinput[inputLen] = '\0';
		}
		else if (inputLen < userMax) {
			qinput[inputLen] = c;
			qinput[inputLen + 1] = '\0';
			inputLen++;
		}
	}
	const Vector2 mp = getCursorPosition();
	deltaMP = Vector2_Sub(mp, prevMP);
	prevMP = mp;
	qgSetCamera((QV3){3,2,6}, (QV3){0,0,0}, 60);
	if (actualScene.update) actualScene.update();
}
int initEngineProject(Scene scene) {
	if (!qsInit()) return 1;
	qgSetBackground(CLR_RGB(.0f, .0f, .0f));
	actualScene = scene;
	const int qgpu = qgpuCreate(QEP_START_WIDTH, QEP_START_HEIGHT, QEP_NAME, qeInit, qeUpdate);
	qsClose();
	return qgpu;
}
void setScene(Scene scene) { actualScene = scene; }
// = = = = = CAMERA = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static Camera _camera;
uint getWidth() { return qgGetWidth(); }
uint getHeight() { return qgGetHeight(); }
Camera getCamera() { return _camera; }
void setCamera(const Camera camera) { _camera = camera; }
void setCameraPos(const Vector3 position) { _camera.position = position; }
void setCameraRot(const Vector3 rotation) { _camera.position = rotation; }
void setCameraScale(const Vector3 scale) { _camera.position = scale; }
// = = = = = GRAPHIC = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void addLight(const Vector3 position, const float range, const float power) { qgAddLight(*(QV3*)&position, range, power); }
void setLayer(const uint layer) { qgSetLayerUI(layer); }

static float b01(const byte v) { return v / 255.0f; }
static void setRot(const Vector3 position, const Vector3 rotation) {
	qgSetRotationPivot(*(QV3*)&position);
	qgSetRotation(*(QV3*)&rotation);
}
static ColorRGBA vte(const Color c) { return (ColorRGBA){b01(c.r), b01(c.g), b01(c.b), b01(c.a)}; }

void drawTriangle(const Vector3 posA, const Vector3 posB, const Vector3 posC, const Color color) { qgAddTriangle(*(QV3*)&posA, *(QV3*)&posB, *(QV3*)&posC, vte(color)); }
void drawRect(const Vector2 position, const Vector2 size, const qeAlignMode align, const Color color) {
	const Vector2 p = getPos(position, align);
	qgAddRect(*(QV2*)&p, *(QV2*)&size, vte(color));
}
void drawCircle(const Vector2 position, const uint segments, const float radius, const qeAlignMode align, const Color color) {
	const Vector2 p = getPos(position, align);
	qgAddCircle(*(QV2*)&p, segments, radius, vte(color));
}
void drawText(const char* text, const Vector2 position, const float fontSize, const qeAlignMode align, const Color color) {
	const Vector2 p = getPos(position, align);
	qgSetFontData(fontSize, QGPU_FONT_STYLE_REGULAR, vte(color));
	qgAddText(*(QV2*)&p, text);
}

void drawButton(uint8* value, const Vector2 position, const Vector2 size, const qeAlignMode align, const Color color) {
	const Vector2 p = getPos(position, align);
	qgAddButton(value, *(QV2*)&p, *(QV2*)&size, vte(color));
}

void drawBox(const Vector3 position, const Vector3 rotation, const Vector3 size, const Color color) {
	setRot(position, rotation);
	qgAddBox(*(QV3*)&position, *(QV3*)&size, vte(color));
}
// = = = = = AUDIO = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void convertAudio(const char* qsr_path, const char* qs_path) { qsConvert(qsr_path, qs_path); }
uint loadAudio(const char* path) { return qsOpen(path); }
void playAudio(const uint audioID, const uint8 volume, const float speed) { qsPlay(audioID, volume, speed); }
// = = = = = INPUT = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void clearInput() {
	qinput[0] = '\0';
	inputLen = 0;
}
void enableInput() { inputOn = true; }
void disableInput() { inputOn = false; }
state getInputState() { return inputOn; }
void setMaxInput(uint max) {
	if (max > MAX_INPUT) {
		userMax = MAX_INPUT;
		return;
	}
	userMax = max;
}
void getInput(char* buffor, const uint length) { qCopy(buffor, length, qinput); }
state getKeyState(const uint keyCode) {
	if (keyCode == KEY_CAPSLOCK) return isCaps;
	return qgGetKey(keyCode);
}
state onKeyDown(const uint keyCode) { return qgOnKey(keyCode); }
typedef struct { char normal, shifted; } KeyMap;
static const KeyMap keymap[] = {
	{ '0', ')' }, { '1', '!' }, { '2', '@' }, { '3', '#' }, { '4', '$' }, { '5', '%' }, { '6', '^' },
	{ '7', '&' }, { '8', '*' }, { '9', '(' }, { '\'', '"'}, { ',', '<' }, { '-', '_' }, { '.', '>'  },
	{ '/', '?' }, { ';', ':' }, { '=', '+' }, { '[', '{'  }, { '\\', '|'}, { ']', '}' }, { '`', '~' }
};
static const uint mapSize = sizeof(keymap) / sizeof(KeyMap);
char getPressedKey() {
	if (onKeyDown(KEY_SPACE)) return ' ';
	if (onKeyDown(KEY_BACKSPACE)) return '\b';
	if (onKeyDown(KEY_ENTER)) return '\n';
	const state caps = getKeyState(KEY_CAPSLOCK) != 0,
	shift = getKeyState(KEY_LSHIFT) != 0,
	isUpper = caps ^ shift;
	for (uint8 key = KEY_A; key <= KEY_Z; key++) if (onKeyDown(key)) return isUpper ? key : (key + 32);
	for (uint8 i = 0; i < mapSize; i++) if (onKeyDown(keymap[i].normal)) return shift ? keymap[i].shifted : keymap[i].normal;
	return 0;
}
state getMouseButton(const uint mouseKey) { return qgGetMouse(mouseKey); }
state onMouseDown(const uint mouseKey) { return qgOnMouse(mouseKey); }
Vector2 getCursorPosition() {
	float x, y;
	qgGetMousePos(&x, &y);
	return V2(x, y);
}
