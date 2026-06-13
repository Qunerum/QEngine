#include "../lib/qgpu.h"
#include "../include/ui.h"
#include "../include/main.h"

QColor normalText = (QColor){.9, .9, .9, 1};
QColor sideTitleText = (QColor){.7, .7, .7, 1};

QColor toolBarBack = (QColor){.3, .3, .3, 1};

QColor sideTitle = (QColor){.25, .25, .25, 1};
QColor sideBack = (QColor){.15, .15, .15, 1};

QColor btnLight = (QColor){.45, .45, .45, 1};
QColor btnHover = (QColor){.36, .36, .36, 1};
QColor btnPress = (QColor){.4, .4, .4, 1};

int w = 0, h = 0,
    toolBtns = 0, coderDelay = 0;
float toolBtnW = 140, toolH = 20,
        SceneW = 200, InspectorW = 200,
        coderFilesH = 20, coderTextScale = 1.25;

void drawWindow(int win) {
    float xL = -w+SceneW, xR = w-SceneW,
            yU = h-toolH, yB = -h;
    switch (win) {
        case 0: /* Viewport */ break;
        case 1: // Coder
            if (coderDelay >= CODER_CURSOR_DELAY) coderDelay = 0;
            drawRect(0, yU-coderFilesH/2, w*2-SceneW-InspectorW, coderFilesH, sideBack);
            drawText(xL+5, yU-5, "test.c", coderFilesH*0.075, sideTitleText);
            for (int i=0;i<coder.lineCount;i++) { drawText(xL+5, yU-coderFilesH-5-(i*CHAR_SIZE*1.25*coderTextScale), coder.lines[i], coderTextScale, normalText); }
            if (coderDelay >= CODER_CURSOR_DELAY/2)
                drawRect(xL+5+(CHAR_SIZE*coderTextScale/2)+(coder.cursorX*CHAR_SIZE*1.25*coderTextScale), yU-coderFilesH-5-CHAR_SIZE*coderTextScale, CHAR_SIZE*coderTextScale, 2, WHITE);
            coderDelay++;
            break;
    }
}

void updateUI(int win) {
    w = getWidth() / 2; h = getHeight() / 2; toolBtns = 0;
    drawRect(0, 0, w*2, h*2, BLACK); // Background
    drawWindow(win);

    drawRect(0, h-toolH/2, w*2, toolH, toolBarBack); // Tool bar

    drawRect(-w+SceneW/2, -toolH/2, SceneW, h*2-toolH, sideBack); // Scene
    drawRect(-w+SceneW/2, h-toolH*1.5, SceneW, toolH, sideTitle);
    drawText(-w+5, h-toolH-5, "Scene", toolH*0.075, sideTitleText);

    drawRect(w-InspectorW/2, -toolH/2, InspectorW, h*2-toolH, sideBack); // Inspector
    drawRect(w-InspectorW/2, h-toolH*1.5, InspectorW, toolH, sideTitle);
    drawText(w-InspectorW+5, h-toolH-5, "Inspector", toolH*0.075, sideTitleText);
}
void drawToolButton(char* text, int choosed, void (*action)()) {
    if (drawButton(-w+toolBtnW/2+toolBtnW*toolBtns, h-toolH/2, toolBtnW, toolH, choosed ? btnLight : toolBarBack, choosed ? btnLight : btnHover, choosed ? btnLight : btnPress) == 1 && !choosed) action();
    drawText(-w+5+toolBtnW*toolBtns, h-5, text, toolH*0.075, normalText);
    toolBtns++;
}

