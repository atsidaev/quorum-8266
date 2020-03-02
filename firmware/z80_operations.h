#ifndef Z80_OPERATIONS_H
#define Z80_OPERATIONS_H

#include "lib/Z80/z80.h"
#include "z80filedecoder.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ROMSIZE 16384 //49152
#define RAMSIZE 49152
extern unsigned char RAM[];
extern const unsigned char ROM[];
extern unsigned char KEY[];
extern unsigned char KEMPSTONJOYSTICK;

//used by the .z80 upload to set initial status
boolean SetZ80(Z80 *R, struct z80fileheader * header);
boolean GetZ80(Z80 *R, struct z80fileheader * header);

#ifdef __cplusplus
}
#endif

#endif
