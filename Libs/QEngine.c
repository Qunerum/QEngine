#define QEngine_Input
#define QEngine_Math
#define QEngine_Memory
#define QEngine_IO
#define QEngine_Text
#include "QEngine.h"

#include "qgpu.h"
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
	long size = ftell(file);
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
	int result = fputs(text, file);
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
state qCopyFile(const char* srcPath, const char* dstPath) {
	if (!srcPath || !dstPath) return false;
	FILE* src = fopen(srcPath, "rb");
	if (!src) return false;
	FILE* dst = fopen(dstPath, "wb");
	if (!dst) {
		fclose(src);
		return false;
	}
	char buffer[8192];
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
	state written = vsnprintf(to, length, format, args);
	va_end(args);
	return written;
}
static void (*userInit)() = NULL, (*userUpdate)() = NULL;

#if IS_EDITOR
static state isLight = false;
static Color c1, c2;
static uint8 window = 0;
#endif
static Vector3 getPos(const Vector3 pos, const qeAlignMode align) {
	Vector3 v = pos;
	float w = qgGetWidth() / 2.0f, h = qgGetHeight() / 2.0f;
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
static state mob(const Vector3 pos, const Vector2 size, const qeAlignMode align) {
	Vector2 m = getCursorPosition();
	Vector3 p = getPos(pos, align);
	float a = p.x - size.x / 2.0f, b = p.x + size.x / 2.0f, c = p.y - size.y / 2.0f, d = p.y + size.y / 2.0f;
	return a <= m.x && m.x <= b && c <= m.y && m.y <= d;
}

static void qeInit() {
	print("Application was made in QEngine v%i.%i.%i\n", QENGINE_VERSION_MAJOR, QENGINE_VERSION_MINOR, QENGINE_VERSION_PATCH);
#if IS_EDITOR
	setDrawingMode(UI);
	c1 = isLight ? Clr(100) : Clr(60);
	c2 = isLight ? Clr(120) : Clr(80);
	qgSetBackground(0.1f, 0.1f, 0.1f);
#else
	if (userInit) userInit();
#endif
}
static state isCaps = false, inputOn = false;
static char qinput[MAX_INPUT + 1];
static uint inputLen = 0, userMax = MAX_INPUT;
#if IS_EDITOR
static Vector2 viewPos = V2_Zero;
static const Vector2 blockSize = V2(200, 50);
static void drawCodeBlock(const char* title, const Vector2 position, const Color color) {
	Vector3 pos = Vector3_Add(V3(-position.x, position.y), V3(viewPos.x, viewPos.y));
	drawRect(pos, V3_Zero, blockSize, Bottom_Right, color);
	drawText(title, Vector3_Sub(pos, V3(blockSize.x / 2.0f - 5, -blockSize.y / 2.0f + 5)), V3_Zero, 1.6f, Bottom_Right, Color_White);
}
static float connectF(const float x, const float p) {
	if (x <= 0.0f) return 0.0f;
	if (x >= 1.0f) return 1.0f;
	const float k = (x - 1.0f);
	return x * (1.0f + k * (p - 1.0f) + 0.5f * k * k * (p - 1.0f) * (p - 2.0f));
}
static void drawCodeConnect(Vector2 p1, Vector2 p2, const Color c1, const Color c2) {
	p1.x -= 100 + viewPos.x;
	p1.y += viewPos.y;
	p2.x += 100 - viewPos.x;
	p2.y += viewPos.y;
	const uint8 steps = qClamp_i((uint8)(qDist(p1, p2) / 16.0f), 0, UINT8_MAX);
	if (steps < 2) return;
	float dy = qAbs(p1.y - p2.y);
	if (dy > 300.0f) dy = 300.0f;
	float centerDensity = qMap(dy, 0, 300, 1, 0.6f);
	Vector3 vc1 = V3(c1.r, c1.g, c1.b), vc2 = V3(c2.r, c2.g, c2.b);
	for (uint8 i = 0; i <= steps; i++) {
		const float t = (float)i / (float)steps;
		float t_stepped;
		if (t < 0.5f) t_stepped = 0.5f * connectF(2.0f * t, centerDensity); else t_stepped = 1.0f - 0.5f * connectF(2.0f * (1.0f - t), centerDensity);
		const float smoothY = (t_stepped < 0.5f) ? 0.5f * connectF(2.0f * t_stepped, 3) : 1.0f - 0.5f * connectF(2.0f * (1.0f - t_stepped), 3),
		px = p1.x + (p2.x - p1.x) * t_stepped,
		py = p1.y + (p2.y - p1.y) * smoothY;
		const Vector3 c = qLerp(vc2, vc1, (float)(steps - i) / (float)steps);
		drawRect(V3(-px, py), V3_Zero, V2(5, 5), Bottom_Right, Clr((byte)c.x, (byte)c.y, (byte)c.z));
	}
}
#endif
static void qeUpdate() {
// = = = = = INPUT = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
	if (onKeyDown(KEY_CAPSLOCK)) isCaps = !isCaps;
	char c = getPressedKey();
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
// = = = = = EDITOR = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
#if IS_EDITOR
	uint w = getWidth(), h = getHeight();
	// Main
	// blocks (test)
	const Vector2 p1 = V2(800, 600);
	static Vector2 p2 = V2(300, 200);

	drawCodeConnect(p1, p2, Clr(160, 20, 20), Clr(20, 160, 20));
	drawCodeBlock("Test block #1", p1, Clr(160, 20, 20));
	static const float spd = 2;
	if (getKeyState(KEY_W)) p2.y += spd;
	if (getKeyState(KEY_S)) p2.y -= spd;
	if (getKeyState(KEY_A)) p2.x += spd;
	if (getKeyState(KEY_D)) p2.x -= spd;

	if (getKeyState(KEY_UP)) viewPos.y += spd;
	if (getKeyState(KEY_DOWN)) viewPos.y -= spd;
	if (getKeyState(KEY_LEFT)) viewPos.x -= spd;
	if (getKeyState(KEY_RIGHT)) viewPos.x += spd;

	drawCodeBlock("Test block #2", p2, Clr(20, 160, 20));

	// Up bar
	drawRect(V3(0, -15), V3_Zero, V2(w, 30), Top, c2);
	// Buttons
	for (int i = 0; i < 5; i++) {
		if (drawButton(V3(102 + 204 * i, -15), V3_Zero, V2(200, 26), Top_Left, window == i ? Clr(50) : c1, Clr(40), Clr(30))) { window = i; }
		drawText("test.qeb", V3(5 + 204 * i, -5), V3_Zero, 1.5f, Top_Left, Color_White);
	}
	// Left panel
	drawRect(V3(150, -15), V3_Zero, V2(300, h - 30), Left, c1);
#else
	if (userUpdate) userUpdate();
#endif
	qgLogVertices();
}
int initEngineProject(void (*initFunc)(), void (*updateFunc)()) {
	if (!qsInit()) return 1;
	qgSetBackground(0, 0, 0);
	char title[MAX_NAME_LENGTH];
	if (IS_EDITOR) snprintf(title, sizeof(title), "QEngine %i.%i.%i Block Code Editor | %s %s", QENGINE_VERSION_MAJOR, QENGINE_VERSION_MINOR, QENGINE_VERSION_PATCH, QEP_NAME, QEP_VERSION);
	else snprintf(title, sizeof(title), "%s %s", QEP_NAME, QEP_VERSION);
	if (!IS_EDITOR) {
		userInit = initFunc;
		userUpdate = updateFunc;
	}
	qgpuCreate(QEP_START_WIDTH, QEP_START_HEIGHT, title, qeInit, qeUpdate);
	qsClose();
	return 0;
}
// = = = = = CAMERA = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
static Camera _camera;
void setDrawingMode(const qeDrawingMode mode) {
	switch (mode) {
		case World: qgSetRenderType(QGPU_RENDER_TYPE_LIGHT); break;
		case UI: qgSetRenderType(QGPU_RENDER_TYPE_NO_LIGHT); break;
	}
}
uint getWidth() { return qgGetWidth(); }
uint getHeight() { return qgGetHeight(); }
Camera getCamera() { return _camera; }
void setCamera(const Camera camera) { _camera = camera; }
void setCameraPos(const Vector3 position) { _camera.position = position; }
void setCameraRot(const Vector3 rotation) { _camera.position = rotation; }
void setCameraScale(const Vector3 scale) { _camera.position = scale; }
// = = = = = GRAPHIC = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void addLight(const Vector3 position, const float range, const float intense) { qgAddLight(position.x, position.y, position.z, range, intense); }

static float byteTo01(const byte v) { return v / 255.0f; }
static void setRot(const Vector3 position, const Vector3 rotation) {
	qgSetRotationPivot(position.x, position.y, position.z);
	qgSetRotation(rotation.x, rotation.y, rotation.z);
}

void drawTriangle(const Vector3 posA, const Vector3 posB, const Vector3 posC, const Color color) {
	qgAddTriangle(posA.x, posA.y, posA.z, posB.x, posB.y, posB.z, posC.x, posC.y, posC.z, byteTo01(color.r), byteTo01(color.g), byteTo01(color.b), byteTo01(color.a));
}
void drawRect(const Vector3 position, const Vector3 rotation, const Vector2 size, const qeAlignMode align, const Color color) {
	setRot(position, rotation);
	Vector3 p = getPos(position, align);
	qgAddRect(p.x, p.y, p.z, size.x, size.y, byteTo01(color.r), byteTo01(color.g), byteTo01(color.b), byteTo01(color.a));
}
void drawCircle(const Vector3 position, const Vector3 rotation, const uint segments, const float radius, const qeAlignMode align, const Color color) {
	setRot(position, rotation);
	Vector3 p = getPos(position, align);
	qgAddCircle(p.x, p.y, p.z, segments, radius, byteTo01(color.r), byteTo01(color.g), byteTo01(color.b), byteTo01(color.a));
}
void drawText(const char* text, const Vector3 position, const Vector3 rotation, const float fontSize, const qeAlignMode align, const Color color) {
	setRot(position, rotation);
	Vector3 p = getPos(position, align);
	qgSetFontData(fontSize, QGPU_FONT_STYLE_REGULAR, byteTo01(color.r), byteTo01(color.g), byteTo01(color.b), byteTo01(color.a));
	qgAddText(p.x, p.y, p.z, text);
}

state drawButton(const Vector3 position, const Vector3 rotation, const Vector2 size, const qeAlignMode align, const Color clrBase, const Color clrHover, const Color clrPress) {
	setRot(position, rotation);
	Vector3 p = getPos(position, align);
	state hover = mob(position, size, align);
	Color c = hover ? getMouseButton(LMB) ? clrPress : clrHover : clrBase;
	qgAddRect(p.x, p.y, p.z, size.x, size.y, byteTo01(c.r), byteTo01(c.g), byteTo01(c.b), byteTo01(c.a));
	return hover && onMouseDown(LMB);
}

void drawBox(const Vector3 position, const Vector3 rotation, const Vector3 size, const Color color) {
	setRot(position, rotation);
	qgAddBox(position.x, position.y, position.z, size.x, size.y, size.z, byteTo01(color.r), byteTo01(color.g), byteTo01(color.b), byteTo01(color.a));
}
void drawSphere(const Vector3 position, const Vector3 rotation, const uint rings, const uint sectors, const float radius, const Color color) {
	setRot(position, rotation);
	qgAddSphere(position.x, position.y, position.z, radius, rings, sectors, byteTo01(color.r), byteTo01(color.g), byteTo01(color.b), byteTo01(color.a));
}
// = = = = = AUDIO = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void convertAudio(const char* qsr_path, const char* qs_path) { qsConvert(qsr_path, qs_path); }
uint loadAudio(const char* path) { return qsOpen(path); }
void playAudio(const uint audioID, const uint8 volume, const float speed) { qsPlay(audioID, volume, speed); }
// = = = = = INPUT = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void clearInput() { qinput[0] = '\0'; inputLen = 0; }
void enableInput() { inputOn = true; }
void disableInput() { inputOn = false; }
state getInputState() { return inputOn; }
void setMaxInput(uint max) { if (max > MAX_INPUT) { userMax = MAX_INPUT; return; } userMax = max; }
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
	state caps = getKeyState(KEY_CAPSLOCK) != 0,
	shift = getKeyState(KEY_LSHIFT) != 0,
	isUpper = caps ^ shift;
	for (uint key = KEY_A; key <= KEY_Z; key++) if (onKeyDown(key)) return isUpper ? key : (key + 32);
	for (uint i = 0; i < mapSize; i++) if (onKeyDown(keymap[i].normal)) return shift ? keymap[i].shifted : keymap[i].normal;
	return 0;
}
state getMouseButton(const uint mouseKey) { return qgGetMouse(mouseKey); }
state onMouseDown(const uint mouseKey) { return qgOnMouse(mouseKey); }
Vector2 getCursorPosition() {
	float x, y;
	qgGetMousePos(&x, &y);
	return V2(x, y);
}
