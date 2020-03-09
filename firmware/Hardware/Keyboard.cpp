#include <stddef.h>
#include "Keyboard.h"

unsigned char KEY[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char KEMPSTONJOYSTICK = 0x00;
//0..7 used for zx spectrum keyboard emulation
//[8] used for kempston joystik and special keys
//0=right
//1=left
//2=down
//3=up
//4=button2
//5=button1
//6=ESC key
//7=

//to simulate keyboard with 6 joy keys
unsigned char *pJoyKeyAdd[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
unsigned char  pJoyKeyVal[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
//up down left right f1 f2
