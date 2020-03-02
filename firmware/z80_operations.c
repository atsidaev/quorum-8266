#include <pgmspace.h>

#include "Zxsound.h"
#include "z80_operations.h"

#ifdef __C99__
#define INLINE static inline
#else
#define INLINE static __inline
#endif

byte RdZ80(word16 A)        {
  //return (A < ROMSIZE ?  ROM[A]  : RAM[A - ROMSIZE]);
  return (A < ROMSIZE ? pgm_read_byte(ROM + A) : RAM[A - ROMSIZE]);
}

void WrZ80(word16 A, byte V) {
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

boolean SetZ80(Z80 *R, struct z80fileheader * header)
{

  ResetZ80(R);

  const uint8_t *ptr;
  uint8_t wert1, wert2;
  uint8_t flag_version = 0;

  uint16_t header_len;
  uint16_t akt_adr;

  // pointer auf Datenanfang setzen
  ptr = header;

  R->AF.B.h = *(ptr++); // A [0]
  R->AF.B.l = *(ptr++); // F [1]
  R->BC.B.l = *(ptr++); // C [2]
  R->BC.B.h = *(ptr++); // B [3]
  R->HL.B.l = *(ptr++); // L [4]
  R->HL.B.h = *(ptr++); // H [5]

  // PC [6+7]
  wert1 = *(ptr++);
  wert2 = *(ptr++);
  R->PC.W = (wert2 << 8) | wert1;
  if (R->PC.W == 0x0000) {
    return false;

  }

  // SP [8+9]
  wert1 = *(ptr++);
  wert2 = *(ptr++);
  R->SP.W = (wert2 << 8) | wert1;

  R->I = *(ptr++); // I [10]
  R->R = *(ptr++); // R [11]

  // Comressed-Flag und Border [12]
  wert1 = *(ptr++);

  wert2 = ((wert1 & 0x0E) >> 1);

  zxDisplayBorderSet(wert2);


  R->DE.B.l = *(ptr++); // E [13]
  R->DE.B.h = *(ptr++); // D [14]
  R->BC1.B.l = *(ptr++); // C1 [15]
  R->BC1.B.h = *(ptr++); // B1 [16]
  R->DE1.B.l = *(ptr++); // E1 [17]
  R->DE1.B.h = *(ptr++); // D1 [18]
  R->HL1.B.l = *(ptr++); // L1 [19]
  R->HL1.B.h = *(ptr++); // H1 [20]
  R->AF1.B.h = *(ptr++); // A1 [21]
  R->AF1.B.l = *(ptr++); // F1 [22]
  R->IY.B.l = *(ptr++); // Y [23]
  R->IY.B.h = *(ptr++); // I [24]
  R->IX.B.l = *(ptr++); // X [25]
  R->IX.B.h = *(ptr++); // I [26]

  // Interrupt-Flag [27]
  wert1 = *(ptr++);
  if (wert1 != 0) {
    // EI
    R->IFF |= IFF_2 | IFF_EI;
  }
  else {
    // DI
    R->IFF &= ~(IFF_1 | IFF_2 | IFF_EI);
  }
  wert1 = *(ptr++); // nc [28]
  // Interrupt-Mode [29]
  wert1 = *(ptr++);
  if ((wert1 & 0x01) != 0) {
    R->IFF |= IFF_IM1;
  }
  else {
    R->IFF &= ~IFF_IM1;
  }
  if ((wert1 & 0x02) != 0) {
    R->IFF |= IFF_IM2;
  }
  else {
    R->IFF &= ~IFF_IM2;
  }




  R->ICount   = R->IPeriod;
  R->IRequest = INT_NONE;
  R->IBackup  = 0;


  return true;
}


boolean GetZ80(Z80 *R, struct z80fileheader * header)
{

    uint8_t *ptr;
  uint8_t wert1, wert2;
  uint8_t flag_version = 0;

  uint16_t header_len;
  uint16_t akt_adr;

  // pointer auf Datenanfang setzen
  ptr = header;

  *(ptr++) = R->AF.B.h; // A [0]
  *(ptr++) = R->AF.B.l; // F [1]
  *(ptr++) = R->BC.B.l; // C [2]
  *(ptr++) = R->BC.B.h; // B [3]
  *(ptr++) = R->HL.B.l; // L [4]
  *(ptr++) = R->HL.B.h; // H [5]


  // PC [6+7]
  *(ptr++) = R->PC.W & 0xff;
  *(ptr++) = (R->PC.W >> 8);

  // SP [8+9]
  *(ptr++) = R->SP.W & 0xff;
  *(ptr++) = (R->SP.W >> 8);


  *(ptr++) = R->I; // I [10]
  *(ptr++) = R->R; // R [11]

  // Comressed-Flag und Border [12]
  *(ptr++) = (R->R & 128 ? 1 : 0) + //bit 7 of R register
             (zxDisplayBorderGet() << 1) +
             32;//compressed
  //          12      1       Bit 0  : Bit 7 of the R-register
  //                        Bit 1-3: Border colour
  //                        Bit 4  : 1=Basic SamRom switched in
  //                        Bit 5  : 1=Block of data is compressed
  //                        Bit 6-7: No meaning



  *(ptr++) = R->DE.B.l; // E [13]
  *(ptr++) = R->DE.B.h; // D [14]
  *(ptr++) = R->BC1.B.l; // C1 [15]
  *(ptr++) = R->BC1.B.h; // B1 [16]
  *(ptr++) = R->DE1.B.l; // E1 [17]
  *(ptr++) = R->DE1.B.h; // D1 [18]
  *(ptr++) = R->HL1.B.l; // L1 [19]
  *(ptr++) = R->HL1.B.h; // H1 [20]
  *(ptr++) = R->AF1.B.h; // A1 [21]
  *(ptr++) = R->AF1.B.l; // F1 [22]
  *(ptr++) = R->IY.B.l; // Y [23]
  *(ptr++) = R->IY.B.h; // I [24]
  *(ptr++) = R->IX.B.l; // X [25]
  *(ptr++) = R->IX.B.h; // I [26]

  // Interrupt-Flag [27]
  *(ptr++) =(IFF_2 | IFF_EI)?1:0;
  ptr++;
  
  // Interrupt-Mode [29]
   *(ptr++)=(R->IFF & IFF_IM1)?1:0+
            (R->IFF & IFF_IM2)?2:0+ 
            64//kempston joystik
            ;

  return true;
}
