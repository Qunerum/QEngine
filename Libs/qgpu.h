/*
 * Copyright (c) 2026 Qunerum
 *
 * This file is part of QGPU.
 * This software is licensed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef QGPU_H
#define QGPU_H
#include <stdint.h>

#define QGPU_VERSION_MAJOR 2
#define QGPU_VERSION_MINOR 11
#define QGPU_VERSION_PATCH 0

#define __QGPU_SS(x) #x
#define __QGPU_S(x) __QGPU_SS(x)
#define QGPU_VERSION_TEXT __QGPU_S(QGPU_VERSION_MAJOR) "." __QGPU_S(QGPU_VERSION_MINOR) "." __QGPU_S(QGPU_VERSION_PATCH)
// !===== CONFIGURATION ==================================================================================================================================================!
#define MAX_VERTICES     65536 // (2^16) Max vertices in one frame
#define MAX_LIGHTS       1024  // (2^10) Max lights in one frame
#define MAX_UI_VERTICES  32768  // Max 2D/UI vertices in one frame
#define VSYNC            1
#define SHADOW_MAP_SIZE  4096  // resolution of the (single) shadow depth map
// !===== Structs ========================================================================================================================================================!
typedef unsigned int uint;
typedef struct { float x, y; } Vector2;
typedef struct { float x, y, z; } Vector3;
typedef struct { float r, g, b; } ColorRGB;
typedef struct { float r, g, b, a; } ColorRGBA;
#define CLR_RGB(r, g, b) (ColorRGB){r, g, b}
#define CLR_RGBA(r, g, b, a) (ColorRGBA){r, g, b, a}
#ifndef NULL
#define NULL ((void *)0)
#endif
// !===== Console ========================================================================================================================================================!
void qgSetColor(const uint8_t color);
void qgRestoreColor();
#define QGPU_STYLE_REGULAR 0
#define QGPU_STYLE_BOLD 1
#define QGPU_STYLE_ITALIC 3
#define QGPU_STYLE_UNDERLINE 4
#define QGPU_STYLE_STRIKETHROUGH 9
void qgSetStyle(const uint8_t style);
void qgPrint(const char* format, ...);
void qgLog(const char* format, ...);
void qgLogVertices();
void qgWarn(const char* format, ...);
void qgError(const char* format, ...);

typedef enum __attribute__((__packed__)) {
	QGPU_SHOW_BANNER = 0,
	QGPU_SHOW_MADE_WITH_QGPU,
	QGPU_SHOW_INFO,
	QGPU_SHOW_COLORS,
	QGPU_SHOW_LOGS
} QgShower;
void qgSetShow(const QgShower shower, const uint8_t state);
// !===== QGPU ===========================================================================================================================================================!
typedef enum __attribute__((__packed__)) {
	QGPU_SETTINGS_AMBIENT_OCCLUSION = 0,
	QGPU_SETTINGS_MSAA_LEVEL,
	QGPU_SETTINGS_SHADOWS
} QgSetting;

void qgSetGraphicsSetting(const QgSetting setting, const uint8_t value);
// !===== Init
int qgpuCreate(const uint width, const uint height, const char* title, void (*initFunc)(), void (*updateFunc)());
// !===== Window
float qgGetFPS();
// !===== Drawing ========================================================================================================================================================!
#define QGPU_RGB_WHITE         ((ColorRGB){1.0f,1.0f,1.0f})
#define QGPU_RGB_GRAY          ((ColorRGB){0.5f,0.5f,0.5f})
#define QGPU_RGB_DARK_GRAY     ((ColorRGB){.25f,.25f,.25f})
#define QGPU_RGB_BLACK         ((ColorRGB){0.0f,0.0f,0.0f})
#define QGPU_RGB_RED           ((ColorRGB){1.0f,0.0f,0.0f})
#define QGPU_RGB_GREEN         ((ColorRGB){0.0f,1.0f,0.0f})
#define QGPU_RGB_BLUE          ((ColorRGB){0.0f,0.0f,1.0f})
#define QGPU_RGB_YELLOW        ((ColorRGB){1.0f,1.0f,0.0f})
#define QGPU_RGB_CYAN          ((ColorRGB){0.0f,1.0f,1.0f})
#define QGPU_RGB_MAGENTA       ((ColorRGB){1.0f,0.0f,1.0f})
#define QGPU_RGB_ORANGE        ((ColorRGB){1.0f,0.5f,0.0f})
#define QGPU_RGB_PURPLE        ((ColorRGB){0.5f,0.0f,0.5f})
#define QGPU_RGB_PINK          ((ColorRGB){1.0f,.41f,.71f})
#define QGPU_RGB_BROWN         ((ColorRGB){.65f,.16f,.16f})
#define QGPU_RGB_LIME          ((ColorRGB){.75f,1.0f,0.0f})
#define QGPU_RGB_NAVY_BLUE     ((ColorRGB){0.0f,0.0f,0.5f})

#define QGPU_RGBA_TRANSPARENT ((ColorRGBA){0.0f,0.0f,0.0f,0.0f})
#define QGPU_RGBA_WHITE       ((ColorRGBA){1.0f,1.0f,1.0f,1.0f})
#define QGPU_RGBA_GRAY        ((ColorRGBA){0.5f,0.5f,0.5f,1.0f})
#define QGPU_RGBA_DARK_GRAY   ((ColorRGBA){.25f,.25f,.25f,1.0f})
#define QGPU_RGBA_BLACK       ((ColorRGBA){0.0f,0.0f,0.0f,1.0f})
#define QGPU_RGBA_RED         ((ColorRGBA){1.0f,0.0f,0.0f,1.0f})
#define QGPU_RGBA_GREEN       ((ColorRGBA){0.0f,1.0f,0.0f,1.0f})
#define QGPU_RGBA_BLUE        ((ColorRGBA){0.0f,0.0f,1.0f,1.0f})
#define QGPU_RGBA_YELLOW      ((ColorRGBA){1.0f,1.0f,0.0f,1.0f})
#define QGPU_RGBA_CYAN        ((ColorRGBA){0.0f,1.0f,1.0f,1.0f})
#define QGPU_RGBA_MAGENTA     ((ColorRGBA){1.0f,0.0f,1.0f,1.0f})
#define QGPU_RGBA_ORANGE      ((ColorRGBA){1.0f,0.5f,0.0f,1.0f})
#define QGPU_RGBA_PURPLE      ((ColorRGBA){0.5f,0.0f,0.5f,1.0f})
#define QGPU_RGBA_PINK        ((ColorRGBA){1.0f,.41f,.71f,1.0f})
#define QGPU_RGBA_BROWN       ((ColorRGBA){.65f,.16f,.16f,1.0f})
#define QGPU_RGBA_LIME        ((ColorRGBA){.75f,1.0f,0.0f,1.0f})
#define QGPU_RGBA_NAVY_BLUE   ((ColorRGBA){0.0f,0.0f,0.5f,1.0f})

void qgSetBackground(const ColorRGB color);
void qgAddTriangle(const Vector3 p1, const Vector3 p2, const Vector3 p3, const ColorRGBA color);
void qgAddTriangle2D(const Vector2 p1, const Vector2 p2, const Vector2 p3, const ColorRGBA color);
void qgSetLayerUI(const uint layer);
void qgPushClip(const Vector2 position, const Vector2 size);
void qgPopClip();
#ifdef QGPU_SHAPES
void qgAddRect(const Vector2 position, const Vector2 size, const ColorRGBA color);
void qgAddCircle(const Vector2 position, const float radius, const uint segments, const ColorRGBA color);

void qgAddPlane(const Vector3 position, const Vector2 size, const ColorRGBA color);
void qgAddBox(const Vector3 position, const Vector3 size, const ColorRGBA color);
void qgAddCircle3D(const Vector3 position, const float radius, const uint segments, const ColorRGBA color);
void qgAddCone(const Vector3 position, const Vector2 rAh, const uint segments, const ColorRGBA color);
void qgAddCylinder(const Vector3 position, const Vector2 rAh, const uint segments, const ColorRGBA color);
void qgAddHemisphere(const Vector3 position, const float radius, const uint rings, const uint sectors, const ColorRGBA color);
void qgAddSphere(const Vector3 position, const float radius, const uint rings, const uint sectors, const ColorRGBA color);
void qgAddCapsule(const Vector3 position, const Vector2 rAh, const uint rings, const uint sectors, const ColorRGBA color);
#endif
#ifdef QGPU_UI
typedef enum __attribute__((__packed__)) {
	QGPU_BTN_Nothing    = 0b00000000,
	QGPU_BTN_Press      = 0b00000001,
	QGPU_BTN_Release    = 0b00000010,
	QGPU_BTN_MouseEnter = 0b00000100,
	QGPU_BTN_Hover      = 0b00001000,
	QGPU_BTN_MouseLeave = 0b00010000
} QgButtonAction;
void qgAddButton(QgButtonAction* action, const Vector2 position, const Vector2 size, const ColorRGBA clr);
uint8_t qgAddSlider(const float min, const float max, float* value, const Vector2 position, const Vector2 size, const Vector2 handleSize, const ColorRGBA clrFill, const ColorRGBA clrBack, const ColorRGBA clrHandle);
void qgAddToggle(uint8_t* state, const Vector2 position, const Vector2 size, const ColorRGBA clr, const ColorRGBA onClr);
#endif
// !===== Text ===========================================================================================================================================================!
#define QGPU_FONT_STYLE_REGULAR       0b00000000
#define QGPU_FONT_STYLE_BOLD          0b00000001
#define QGPU_FONT_STYLE_ITALIC        0b00000010
#define QGPU_FONT_STYLE_UNDERLINE     0b00000100
#define QGPU_FONT_STYLE_STRIKETHROUGH 0b00001000
#define QGPU_FONT_STYLE_UPPERCASE     0b00010000
#define QGPU_FONT_STYLE_LOWERCASE     0b00100000
void qgLoadFont(const char* path);
void qgSetFontData(const float fontSize, const uint8_t style, const ColorRGBA color);
void qgAddChar(const Vector2 position, const uint32_t c);
void qgAddText(const Vector2 position, const char* text);
// !===== Camera =========================================================================================================================================================!
void qgSetCamera(const Vector3 position, const Vector3 target, const float fovDegrees);
void qgSetCameraUp(const Vector3 up);
void qgSetCameraClip(const float nearZ, const float farZ);
Vector3 qgGetCameraPosition();

void qgSetRotationPivot(const Vector3 pivot);
void qgSetRotation(const Vector3 rotation); // degrees
void qgResetRotation();
// !===== Lights =========================================================================================================================================================!
void qgAddPointLight(const Vector3 position, const float range, const float power, const ColorRGB color);
void qgAddSpotLight(const Vector3 position, const Vector3 direction, const float angleDegrees, const float range, const float power, const ColorRGB color);
void qgAddDirectionalLight(const Vector3 direction, const float power, const ColorRGB color);
// < = = = OBSOLETE (Replaced by: qgAddPointLight) = = = >
void qgAddLight(const Vector3 position, const float range, const float power);
// !===== Input ==========================================================================================================================================================!
uint8_t qgGetKey(const uint key);
uint8_t qgOnKey(const uint key);
uint8_t qgGetMouse(const uint button);
uint8_t qgOnMouse(const uint button);
void qgGetMousePos(float* x, float* y);
typedef enum __attribute__((__packed__)) {
	QGPU_CURSOR_NORMAL = 0,
	QGPU_CURSOR_HIDDEN,
	QGPU_CURSOR_CAPTURED,
	QGPU_CURSOR_DISABLED
} QgCursorState;
void qgSetCursorState(const QgCursorState state);
// !===== Screen =========================================================================================================================================================!
uint qgGetWidth();
uint qgGetHeight();
// !===== Keys ===========================================================================================================================================================!
#define LMB             0
#define RMB             1
#define QKEY_A          65
#define QKEY_B          66
#define QKEY_C          67
#define QKEY_D          68
#define QKEY_E          69
#define QKEY_F          70
#define QKEY_G          71
#define QKEY_H          72
#define QKEY_I          73
#define QKEY_J          74
#define QKEY_K          75
#define QKEY_L          76
#define QKEY_M          77
#define QKEY_N          78
#define QKEY_O          79
#define QKEY_P          80
#define QKEY_Q          81
#define QKEY_R          82
#define QKEY_S          83
#define QKEY_T          84
#define QKEY_U          85
#define QKEY_V          86
#define QKEY_W          87
#define QKEY_X          88
#define QKEY_Y          89
#define QKEY_Z          90
#define QKEY_SPACE      32
#define QKEY_ESCAPE     256
#define QKEY_ENTER      257
#define QKEY_BACKSPACE  259
#define QKEY_LSHIFT     340
#define QKEY_LCTRL      341
#define QKEY_RIGHT      262
#define QKEY_LEFT       263
#define QKEY_DOWN       264
#define QKEY_UP         265

#ifdef QGPU_COLORS
void qgPrintc(uint8_t color, const char* format, ...);
// !===== QPrint ==================================================!
// ANSI escape code using 8-bit color
#define RST     "\033[0m"
#define REGULAR       0
#define BOLD          1
#define BLACK         16
#define WHITE         15
// ===== Light ========================================
#define LIGHT_GRAY    252
#define LIGHT_RED     211
#define LIGHT_GREEN   120
#define LIGHT_YELLOW  227
#define LIGHT_ORANGE  215
#define LIGHT_BLUE    75
#define LIGHT_MAGENTA 207
#define LIGHT_CYAN    51
// ===== Normal ========================================
#define GRAY          244
#define RED           160
#define GREEN         2
#define YELLOW        220
#define ORANGE        208
#define BLUE          27
#define MAGENTA       200
#define CYAN          81
// ===== Dark ========================================
#define DARK_GRAY     241
#define DARK_RED      88
#define DARK_GREEN    22
#define DARK_YELLOW   136
#define DARK_ORANGE   130
#define DARK_BLUE     19
#define DARK_MAGENTA  128
#define DARK_CYAN     68
#endif
#ifdef QGPU_IMPLEMENTATION

#endif
#endif
