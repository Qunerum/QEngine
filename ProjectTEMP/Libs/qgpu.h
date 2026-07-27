#ifndef QGPU_H
#define QGPU_H
#include <stdint.h>

// !===== CONFIGURATION ==================================================================================================================================================!
#define MAX_VERTICES 65536 // (2^16) Max vertices in one frame
#define MAX_LIGHTS   1024  // (2^10) Max lights in one frame
#define VSYNC 0
#define MAX_SPHERE_RINGS 32
#define MAX_SPHERE_SECTORS 32
// !===== Structs ========================================================================================================================================================!
typedef struct { float pos[3]; float color[4]; } QGPU_Vertex;
// !===== Console ========================================================================================================================================================!
#define QGPU_SHOW_BANNER          0
#define QGPU_SHOW_MADE_WITH_QGPU  1
#define QGPU_SHOW_INFO            2
#define QGPU_SHOW_COLORS          3
#define QGPU_SHOW_LOGS            4
void qgSetColor(int color);
void qgRestoreColor();
void qgSetStyle(int style);
void qgPrint(const char* format, ...);
void qgLog(const char* format, ...);
void qgLogVertices();
void qgWarn(const char* format, ...);
void qgError(const char* format, ...);
void qgSetShow(int shower, int state);
// !===== QGPU ===========================================================================================================================================================!
// !===== Init
void qgpuCreate(int width, int height, const char* title, void (*initFunc)(), void (*updateFunc)());
// !===== Window
float qgGetFPS();
// !===== Drawing ========================================================================================================================================================!
void qgSetBackground(float r, float g, float b);
// !===== Rotation
void qgSetRotationPivot(float x, float y, float z);
void qgSetRotation(float rx, float ry, float rz);
void qgResetRotation();
// !===== Vertices & Indices
#define QGPU_RENDER_TYPE_NO_LIGHT 0
#define QGPU_RENDER_TYPE_LIGHT 1
void qgSetRenderType(int type);
uint32_t qgAddVertex(float x, float y, float layer, float r, float g, float b, float a);
void qgAddIndex(uint32_t index);
void qgAddGeometry(QGPU_Vertex* verts, uint32_t vCount, uint32_t* indices, uint32_t iCount);
// !===== Lights
void qgAddLight(float x, float y, float z, float range, float intense);
// !===== Ready 2D
void qgAddTriangle(float p1x, float p1y, float p1z, float p2x, float p2y, float p2z, float p3x, float p3y, float p3z, float r, float g, float b, float a);
void qgAddRect(float px, float py, float pz, float sx, float sy, float r, float g, float b, float a);
void qgAddCircle(float px, float py, float layer, int segments, float radius, float r, float g, float b, float a);
// !===== Ready 3D
void qgAddBox(float px, float py, float pz, float sx, float sy, float sz, float r, float g, float b, float a);
void qgAddSphere(float px, float py, float pz, float radius, int rings, int sectors, float r, float g, float b, float a);
// !===== Screen =========================================================================================================================================================!
int qgGetWidth();
int qgGetHeight();
// !===== Input ==========================================================================================================================================================!
int qgGetKey(int key);
int qgOnKey(int key);
int qgGetMouse(int button);
int qgOnMouse(int button);
void qgGetMousePos(double* x, double* y);
// !===== Text ===========================================================================================================================================================!
#define QGPU_FONT_STYLE_REGULAR 0
#define QGPU_FONT_STYLE_BOLD 1
#define QGPU_FONT_STYLE_ITALIC 2
#define QGPU_FONT_STYLE_BOLD_ITALIC 3
void qgSetFontData(float fontSize, int style, float r, float g, float b, float a);
void qgAddChar(float px, float py, float pz, unsigned char c);
void qgAddText(float px, float py, float pz, const char* text);
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

void qgPrintc(int color, const char* format, ...);
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
#endif
