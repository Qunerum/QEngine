#ifndef QENGINE_LIB_INPUT_H
#define QENGINE_LIB_INPUT_H
#include "QEngine.h"

int getKeyState(int keyCode);
int onKeyDown(int keyCode);
int getMouseButton(int mouseKey);
int onMouseDown(int mouseKey);
Vector2 getCursorPosition();

// ===== Keys ========================================
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
