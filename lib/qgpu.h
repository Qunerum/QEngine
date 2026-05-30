#ifndef QGPU_H
#define QGPU_H

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdint.h>

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

#define TEXTURES 16
extern RawTexture txts[TEXTURES];
#define FILES "Files/"

#define WHITE       (QColor){1.0f, 1.0f, 1.0f, 1.0f}
#define GRAY        (QColor){0.5f, 0.5f, 0.5f, 1.0f}
#define BLACK       (QColor){0.0f, 0.0f, 0.0f, 1.0f}

#define RED         (QColor){1.0f, 0.0f, 0.0f, 1.0f}
#define DARK_RED    (QColor){0.5f, 0.0f, 0.0f, 1.0f}
#define GREEN       (QColor){0.0f, 1.0f, 0.0f, 1.0f}
#define DARK_GREEN  (QColor){0.0f, 0.5f, 0.0f, 1.0f}
#define BLUE        (QColor){0.0f, 0.0f, 1.0f, 1.0f}
#define DARK_BLUE   (QColor){0.0f, 0.0f, 0.5f, 1.0f}

void print(const char* format, ...);

void qgpuCreate(int width, int height, const char* title, void (*updateFunc)());
void drawGeometry(float posX, float posY, QGPU_Vertex* vertices, uint32_t vCount, uint32_t* indices, uint32_t iCount);

void drawRect(float posX, float posY, float sizeX, float sizeY, QColor clr);
void drawTriangle(float posX, float posY, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, QColor clr);
void drawCircle(float posX, float posY, float radius, int segments, QColor clr);

void drawLine(float x1, float y1, float x2, float y2, float thickness, QColor clr);
void drawWireRect(float posX, float posY, float sizeX, float sizeY, float thickness, QColor clr);
void drawWireTriangle(float posX, float posY, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, float thickness, QColor clr);
void drawWireCircle(float posX, float posY, float radius, int segments, float thickness, QColor clr);

void loadTexture(const char* filename, int slot);
void drawTextureScale(float posX, float posY, int textureID, float scale);

int getKey(int key);
int onKey(int key);
int getMouse(int button);
int onMouse(int button);
void getMousePos(double* x, double* y);

int getWidth();
int getHeight();

// UI
int drawButton(float posX, float posY, float width, float height, QColor clr, QColor hoverClr, QColor pressClr);
int drawSlider(float* value, float min, float max, float posX, float posY, float width, float height, float handleW, float handleH, QColor backgroundClr, QColor fillClr, QColor handleClr);
int drawToggle(int* value, float posX, float posY, float width, float height, QColor offClr, QColor onClr);

// KEYBOARD
#define LMB GLFW_MOUSE_BUTTON_LEFT
#define RMB GLFW_MOUSE_BUTTON_RIGHT

#define QKEY_A          GLFW_KEY_A
#define QKEY_B          GLFW_KEY_B
#define QKEY_C          GLFW_KEY_C
#define QKEY_D          GLFW_KEY_D
#define QKEY_E          GLFW_KEY_E
#define QKEY_F          GLFW_KEY_F
#define QKEY_G          GLFW_KEY_G
#define QKEY_H          GLFW_KEY_H
#define QKEY_I          GLFW_KEY_I
#define QKEY_J          GLFW_KEY_J
#define QKEY_K          GLFW_KEY_K
#define QKEY_L          GLFW_KEY_L
#define QKEY_M          GLFW_KEY_M
#define QKEY_N          GLFW_KEY_N
#define QKEY_O          GLFW_KEY_O
#define QKEY_P          GLFW_KEY_P
#define QKEY_Q          GLFW_KEY_Q
#define QKEY_R          GLFW_KEY_R
#define QKEY_S          GLFW_KEY_S
#define QKEY_T          GLFW_KEY_T
#define QKEY_U          GLFW_KEY_U
#define QKEY_V          GLFW_KEY_V
#define QKEY_W          GLFW_KEY_W
#define QKEY_X          GLFW_KEY_X
#define QKEY_Y          GLFW_KEY_Y
#define QKEY_Z          GLFW_KEY_Z

#define QKEY_SPACE      GLFW_KEY_SPACE
#define QKEY_ESCAPE     GLFW_KEY_ESCAPE
#define QKEY_ENTER      GLFW_KEY_ENTER
#define QKEY_BACKSPACE  GLFW_KEY_BACKSPACE
#define QKEY_LSHIFT     GLFW_KEY_LEFT_SHIFT
#define QKEY_LCTRL      GLFW_KEY_LEFT_CONTROL

#define QKEY_UP         GLFW_KEY_UP
#define QKEY_DOWN       GLFW_KEY_DOWN
#define QKEY_LEFT       GLFW_KEY_LEFT
#define QKEY_RIGHT      GLFW_KEY_RIGHT

#endif
