#ifndef QGPU_H
#define QGPU_H

#include <stdint.h>
#define FILES "Files/" // Use this in loadTexture: FILES "texturepath.qgt"
#define TEXTURES 16 // Max textures ( Change if you want :P )

// !===== Structs ==================================================!
typedef struct {
    float pos[2];
    float color[4];
} QGPU_Vertex;
typedef struct { float r, g, b, a; } QColor;
typedef struct {
    unsigned char* pixels;
    int pixelCount;
    int width;
    int height;
} RawTexture;
extern RawTexture txts[TEXTURES];
// !===== Colors ==================================================!
// ===== Grayscale ========================================
#define WHITE       (QColor){1.0f, 1.0f, 1.0f, 1.0f}
#define GRAY        (QColor){0.5f, 0.5f, 0.5f, 1.0f}
#define BLACK       (QColor){0.0f, 0.0f, 0.0f, 1.0f}
// ===== Primary ========================================
#define RED         (QColor){1.0f, 0.0f, 0.0f, 1.0f}
#define GREEN       (QColor){0.0f, 1.0f, 0.0f, 1.0f}
#define BLUE        (QColor){0.0f, 0.0f, 1.0f, 1.0f}
// ===== Dark ========================================
#define DARK_RED    (QColor){0.5f, 0.0f, 0.0f, 1.0f}
#define DARK_GREEN  (QColor){0.0f, 0.5f, 0.0f, 1.0f}
#define DARK_BLUE   (QColor){0.0f, 0.0f, 0.5f, 1.0f}
// !===== Console ==================================================!
void print(const char* format, ...);
// !===== Init ==================================================!
void qgpuCreate(int width, int height, const char* title, void (*updateFunc)());
// !===== Drawing ==================================================!
void drawGeometry(float posX, float posY, QGPU_Vertex* vertices, uint32_t vCount, uint32_t* indices, uint32_t iCount);
// ===== Simple ========================================
void drawRect(float posX, float posY, float sizeX, float sizeY, QColor clr);
void drawTriangle(float posX, float posY, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, QColor clr);
void drawCircle(float posX, float posY, float radius, int segments, QColor clr);
// ===== Wire ========================================
void drawLine(float x1, float y1, float x2, float y2, float thickness, QColor clr);
void drawWireRect(float posX, float posY, float sizeX, float sizeY, float thickness, QColor clr);
void drawWireTriangle(float posX, float posY, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, float thickness, QColor clr);
void drawWireCircle(float posX, float posY, float radius, int segments, float thickness, QColor clr);
// ===== Texture ========================================

void loadTexture(const char* filename, int slot);
void drawTextureScale(float posX, float posY, int slot, float scale);
// ===== UI ========================================
int drawButton(float posX, float posY, float width, float height, QColor clr, QColor hoverClr, QColor pressClr);
int drawSlider(float posX, float posY, float width, float height, float handleW, float handleH, float* value, float min, float max, QColor backgroundClr, QColor fillClr, QColor handleClr);
int drawToggle(float posX, float posY, float width, float height, int* value, QColor offClr, QColor onClr);
// !===== Screen ==================================================!
int getWidth();
int getHeight();
// !===== Keyboard / Mouse ==================================================!
int getKey(int key);
int onKey(int key);
int getMouse(int button);
int onMouse(int button);
void getMousePos(double* x, double* y);
// ===== Keys ========================================
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

#endif
