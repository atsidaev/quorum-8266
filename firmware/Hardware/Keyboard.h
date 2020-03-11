#pragma once

#include <stdint.h>

#include "../z80_operations.h"

extern unsigned char *pJoyKeyAdd[6];
extern unsigned char  pJoyKeyVal[6];

//bit used by keyboard key in zx spectrum
#define BUTTON_6 16
#define BUTTON_7 8
#define BUTTON_8 4
#define BUTTON_9 2
#define BUTTON_0 1

#define BUTTON_Y 16
#define BUTTON_U 8
#define BUTTON_I 4
#define BUTTON_O 2
#define BUTTON_P 1

#define BUTTON_H 16
#define BUTTON_J 8
#define BUTTON_K 4
#define BUTTON_L 2
#define BUTTON_EN 1

#define BUTTON_B 16
#define BUTTON_N 8
#define BUTTON_M 4
#define BUTTON_SS 2
#define BUTTON_SP 1

#define BUTTON_1 1
#define BUTTON_2 2
#define BUTTON_3 4
#define BUTTON_4 8
#define BUTTON_5 16

#define BUTTON_Q 1
#define BUTTON_W 2
#define BUTTON_E 4
#define BUTTON_R 8
#define BUTTON_T 16

#define BUTTON_A 1
#define BUTTON_S 2
#define BUTTON_D 4
#define BUTTON_F 8
#define BUTTON_G 16

#define BUTTON_CS 1
#define BUTTON_Z 2
#define BUTTON_X 4
#define BUTTON_C 8
#define BUTTON_V 16

//kempston joystick
#define BUTTON_LEFT 2
#define BUTTON_RIGHT 1
#define BUTTON_UP 8
#define BUTTON_DOWN 4
#define BUTTON_FIRE2 32
#define BUTTON_FIRE 16
#define BUTTON_ESC 64
