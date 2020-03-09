#include "SerialKeyboard.h"
#include <HardwareSerial.h>
#include "lib/Z80/Z80.h"

extern HardwareSerial Serial;
extern Z80 state;//emulator status

enum escape_code_await_status_t { ESC_NO, ESC_AWAIT_BRACKET, ESC_AWAIT_CODE, ESC_AWAIT_CODE_31, ESC_AWAIT_CODE_32, ESC_AWAIT_7E_TRAILER };
escape_code_await_status_t receiving_esc_seq = ESC_NO;

void serial_keyboard_read()
{
  // Clear keyboard
  KEMPSTONJOYSTICK = 0;
  KEY[0] = KEY[1] = KEY[2] = KEY[3] = KEY[4] = KEY[5] = KEY[6] = KEY[7] = 0xff;

  // Check if any key is pressed
  while (Serial.available())
  {
    char cRead = Serial.read();

    // Process ESC sequences (Fn keys, arrow keys etc)
    if (receiving_esc_seq != ESC_NO) {
        if (receiving_esc_seq == ESC_AWAIT_BRACKET && cRead == '[') 
            receiving_esc_seq = ESC_AWAIT_CODE;
        else if (receiving_esc_seq == ESC_AWAIT_CODE) {
            switch (cRead) {
                case 'A': KEY[4] ^= BUTTON_6; receiving_esc_seq = ESC_NO; break; // UP
                case 'B': KEY[4] ^= BUTTON_8; receiving_esc_seq = ESC_NO; break; // DOWN
                case 'C': KEY[4] ^= BUTTON_7; receiving_esc_seq = ESC_NO; break; // RIGHT
                case 'D': KEY[3] ^= BUTTON_5; receiving_esc_seq = ESC_NO; break; // LEFT
                case 0x31: receiving_esc_seq = ESC_AWAIT_CODE_31; break; // F1..F8
                case 0x32: receiving_esc_seq = ESC_AWAIT_CODE_32; break; // F9..F12
                default: receiving_esc_seq = ESC_NO; break;
            }
        }
        else if (receiving_esc_seq == ESC_AWAIT_CODE_31) {
            receiving_esc_seq = ESC_AWAIT_7E_TRAILER;
        }
        else if (receiving_esc_seq == ESC_AWAIT_CODE_32) {
            switch (cRead) {
                case 0x30:  // F9
                case 0x31:  // F10
                case 0x33:  // F11
                case 0x34: ResetZ80(&state); receiving_esc_seq = ESC_AWAIT_7E_TRAILER; break; // F12 - RESET
                default: receiving_esc_seq = ESC_NO; break;
            }
        }
        else if (receiving_esc_seq == ESC_AWAIT_7E_TRAILER)
        {
            receiving_esc_seq = ESC_NO;
        }
        else
        {
            receiving_esc_seq = ESC_NO;
        }
        continue;
    }
    
    // Read basic 40 keys matrix
    switch (cRead) {
      case 'y': KEY[5] ^= BUTTON_Y; break;
      case 'u': KEY[5] ^= BUTTON_U; break;
      case 'i': KEY[5] ^= BUTTON_I; break;
      case 'o': KEY[5] ^= BUTTON_O; break;
      case 'p': KEY[5] ^= BUTTON_P; break;

      case 'h': KEY[6] ^= BUTTON_H; break;
      case 'j': KEY[6] ^= BUTTON_J; break;
      case 'k': KEY[6] ^= BUTTON_K; break;
      case 'l': KEY[6] ^= BUTTON_L; break;
      case '\r':KEY[6] ^= BUTTON_EN; break;

      case 'b': KEY[7] ^= BUTTON_B; break;
      case 'n': KEY[7] ^= BUTTON_N; break;
      case 'm': KEY[7] ^= BUTTON_M; break;
      case '<': KEY[7] ^= BUTTON_SS; break;
      case ' ': KEY[7] ^= BUTTON_SP; break;

      case '0': KEY[4] ^= BUTTON_0; break;
      case '9': KEY[4] ^= BUTTON_9; break;
      case '8': KEY[4] ^= BUTTON_8; break;
      case '7': KEY[4] ^= BUTTON_7; break;
      case '6': KEY[4] ^= BUTTON_6; break;

      case '1': KEY[3] ^= BUTTON_1; break;
      case '2': KEY[3] ^= BUTTON_2; break;
      case '3': KEY[3] ^= BUTTON_3; break;
      case '4': KEY[3] ^= BUTTON_4; break;
      case '5': KEY[3] ^= BUTTON_5; break;

      case 'q': KEY[2] ^= BUTTON_Q; break;
      case 'w': KEY[2] ^= BUTTON_W; break;
      case 'e': KEY[2] ^= BUTTON_E; break;
      case 'r': KEY[2] ^= BUTTON_R; break;
      case 't': KEY[2] ^= BUTTON_T; break;

      case 'a': KEY[1] ^= BUTTON_A; break;
      case 's': KEY[1] ^= BUTTON_S; break;
      case 'd': KEY[1] ^= BUTTON_D; break;
      case 'f': KEY[1] ^= BUTTON_F; break;
      case 'g': KEY[1] ^= BUTTON_G; break;

      case '>': KEY[0] ^= BUTTON_CS; break;
      case 'z': KEY[0] ^= BUTTON_Z; break;
      case 'x': KEY[0] ^= BUTTON_X; break;
      case 'c': KEY[0] ^= BUTTON_C; break;
      case 'v': KEY[0] ^= BUTTON_V; break;

      case '\x1b': receiving_esc_seq = ESC_AWAIT_BRACKET; break;

      case '|': KEMPSTONJOYSTICK = BUTTON_ESC; break; //esc special key
      default:
                Serial.print("Unknown key: ");
                Serial.print(cRead, HEX);
                break;
    }
  }
}
