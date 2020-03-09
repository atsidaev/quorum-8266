#include <pgmspace.h>

#include "Zxsound.h"
#include "z80_operations.h"
#include "Hardware/Keyboard.h"

byte ICACHE_RAM_ATTR RdZ80(word16 A)        {
  //return (A < ROMSIZE ?  ROM[A]  : RAM[A - ROMSIZE]);
  return (A < ROMSIZE ? pgm_read_byte(ROM + A) : RAM[A - ROMSIZE]);
}

void ICACHE_RAM_ATTR WrZ80(word16 A, byte V) {
//  if (A >= ROMSIZE && A < (ROMSIZE + RAMSIZE)) RAM[A - ROMSIZE] = V;
  if (A >= ROMSIZE) RAM[A - ROMSIZE] = V;
}

void OutZ80(register word16 Port, register byte Value)
{
  if ((Port & 0xFF) == 0xFE) {
    // BorderColor und Ton-Out speichern
    //    out_ram=Value;
    // Ton-Signal schalten
    zxDisplayBorderSet(Value & 7);

    if ( Value & 0x10 ) {
      zxSoundSet(1);
    }
    else {
      zxSoundSet(0);
    }
  }
  if (Port == 0xAAAA) {
    // 1bit Digital-OUT [Test von UB]
    //    ZX_Spectrum.led_mode=1;
    if ((Value & 0x01) == 0) {
      //      LED_RED_PORT->BSRRH = LED_RED_PIN; // Bit auf Lo
    }
    else {
      //      LED_RED_PORT->BSRRL = LED_RED_PIN; // Bit auf Hi
    }
  }
}

byte InZ80(register word16 Port)
{
  byte ret_wert = 0xFF;


  //zxDisplayWriteSerial(Port);


  if ((Port & 0xFF) == 0xFE) {
    // Abfrage der Tastatur und Ton-IN
    if (!(Port & 0x0100)) ret_wert &= KEY[0];
    if (!(Port & 0x0200)) ret_wert &= KEY[1];
    if (!(Port & 0x0400)) ret_wert &= KEY[2];
    if (!(Port & 0x0800)) ret_wert &= KEY[3];
    if (!(Port & 0x1000)) ret_wert &= KEY[4];
    if (!(Port & 0x2000)) ret_wert &= KEY[5];
    if (!(Port & 0x4000)) ret_wert &= KEY[6];
    if (!(Port & 0x8000)) ret_wert &= KEY[7];
  }
  else if ((Port & 0xFF) == 0x1F) {
    // Abfrage vom Kempston-Joystick
    ret_wert = KEMPSTONJOYSTICK;
  }
  else {
    ret_wert = 0x01;
  }


  return (ret_wert);
}


void PatchZ80(register Z80 *R)
{
  // nothing to do
}
