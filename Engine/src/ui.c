#include "../lib/qgpu.h"

QColor normal = (QColor){.2, .2, .2, 1};
QColor btn = (QColor){.175, .175, .175, 1};
QColor btnHover = (QColor){.15, .15, .15, 1};
QColor btnPress = (QColor){.125, .125, .125, 1};

static int w = 0, h = 0,
    toolBtns = 0;
static float toolBtnW = 120, toolH = 20;

void updateUI() {
    w = getWidth() / 2; h = getHeight() / 2; toolBtns = 0;
    drawRect(0, 0, w*2, h*2, BLACK);
    drawRect(0, h-toolH/2, w*2, toolH, normal);
}
void drawToolButton(char* text, void (*action)()) {
    if (drawButton(-w+toolBtnW/2+toolBtnW*toolBtns, h-toolH/2, toolBtnW, toolH, btn, btnHover, btnPress) == 1) { action(); }
    drawText(-w+5+toolBtnW*toolBtns, h-5, text, toolH*0.075, GRAY);
    toolBtns++;
}
