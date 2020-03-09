#include <pgmspace.h>

#include "CommonUI.h"
#include "../z80_operations.h"

void ICACHE_FLASH_ATTR UI_Cls(int col)
{
  memset(RAM, 0, 32 * 192);
  memset(RAM + 32 * 192, col, 32 * 24);
}

//x,y in char coordinates. 0..31 0..23
//return address of bitmap and color
void ICACHE_FLASH_ATTR UI_PixMem(int x, int y, int *bmp, int *col)
{
  y *= 8;
  *bmp = x + (  ((y & (8 + 16 + 32)) << 2) + ((y & 7) << 8) + ((y & (64 + 128)) << 5));
  *col = x + ((192 * 32) + ((y & (255 - 7)) << 2) );
}

//clear a line
void ICACHE_FLASH_ATTR UI_ClearLine(int line, int color)
{
  int bmpoffset;
  int coloffset;
  UI_PixMem(0, line, &bmpoffset, &coloffset);

  for (int i = 0; i < 8; i++) {
    memset(RAM + bmpoffset, 0, 32 );
    bmpoffset += 256;
  }
  memset(RAM + coloffset, color, 32);

}

//print a character
void ICACHE_FLASH_ATTR UI_PrintCh(int x, int y, unsigned char c, int col)
{
  const uint32_t *p = ROM + (c * 8 + 15360);//zx spectrum char set
  int bmpoffset;
  int coloffset;

  unsigned char row;

  UI_PixMem(x, y, &bmpoffset, &coloffset);

  RAM[coloffset] = col;

  for (int i = 0; i < 8; i++)
  {
    row = pgm_read_byte(p);
    p++;
    RAM[bmpoffset] = row;
    bmpoffset += 256;
  }
}

//print a 2x2 character doubling scanlines and horizontal bits
void ICACHE_FLASH_ATTR UI_PrintChBig(int x, int y, unsigned char c, int col)
{
  const uint32_t *p = ROM + (c * 8 + 15360);
  int bmpoffset;
  int coloffset;

  unsigned char row;
  unsigned char row1;
  unsigned char row2;

  UI_PixMem(x, y, &bmpoffset, &coloffset);

  RAM[coloffset] = col;
  RAM[coloffset + 1] = col;
  RAM[coloffset + 32] = col;
  RAM[coloffset + 33] = col;

  for (int i = 0; i < 8; i++)
  {

    if (i == 4)
    {
      UI_PixMem(x, y + 1, &bmpoffset, &coloffset);
    }

    //doubles horizontal pixels
    row = pgm_read_byte(p);
    row1 = 0;
    row2 = 0;
    row1 |= row & 128 ? (128 + 64) : 0;
    row1 |= row & 64 ? (32 + 16) : 0;
    row1 |= row & 32 ? (8 + 4) : 0;
    row1 |= row & 16 ? (2 + 1) : 0;
    row2 |= row & 8 ? (128 + 64) : 0;
    row2 |= row & 4 ? (32 + 16) : 0;
    row2 |= row & 2 ? (8 + 4) : 0;
    row2 |= row & 1 ? (2 + 1) : 0;



    p++;
    RAM[bmpoffset] = row1;
    RAM[bmpoffset + 1] = row2;
    RAM[bmpoffset + 256] = row1;
    RAM[bmpoffset + 1 + 256] = row2;
    bmpoffset += 512;
  }
}

//print a string
void ICACHE_FLASH_ATTR UI_PrintStr(int x, int y, char *str, int col)
{
  for (int i = 0; str[i] && i < 32; i++)
  {
    UI_PrintCh(x++, y, str[i], col);
  }
}

void ICACHE_FLASH_ATTR UI_PrintStrBig(int x, int y, char *str, int col)
{
  for (int i = 0; str[i] && i < 32; i++)
  {
    UI_PrintChBig(x, y, str[i], col);
    x += 2;
  }
}

//print a F string
void ICACHE_FLASH_ATTR UI_PrintFStr(int x, int y, char *str, int col)
{
  char c;
  for (int i = 0; i < 32; i++)
  {
    c = pgm_read_byte(((PGM_P)str ) + i);
    if (c == 0) break;
    UI_PrintCh(x++, y, c, col);
  }
}

void ICACHE_FLASH_ATTR UI_PrintFStrBig(int x, int y, char *str, int col)
{
  char c;
  for (int i = 0; i < 32; i++)
  {
    c = pgm_read_byte(((PGM_P)str ) + i);
    if (c == 0) break;
    UI_PrintChBig(x, y, c, col);
    x += 2;
  }
}
