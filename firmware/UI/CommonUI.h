#ifndef COMMON_UI_H
#define COMMON_UI_H

#define INK_BLACK 0x00
#define INK_BLUE 0x01
#define INK_RED 0x02
#define INK_MAGENTA 0x03
#define INK_GREEN 0x04
#define INK_CYAN 0x05
#define INK_YELLOW 0x06
#define INK_WHITE 0x07
#define PAPER_BLACK (INK_BLACK<<3)
#define PAPER_BLUE (INK_BLUE<<3)
#define PAPER_RED (INK_RED<<3)
#define PAPER_MAGENTA (INK_MAGENTA<<3)
#define PAPER_GREEN (INK_GREEN<<3)
#define PAPER_CYAN (INK_CYAN<<3)
#define PAPER_YELLOW (INK_YELLOW<<3)
#define PAPER_WHITE (INK_WHITE<<3)

#define COLOR_BRIGHT 64
#define COLOR_BLINK 128

#define COLOR_TEXT (COLOR_BRIGHT  | PAPER_BLACK | INK_WHITE)
#define COLOR_FILE (COLOR_BRIGHT  | PAPER_BLACK | INK_YELLOW)
#define COLOR_FILE_SELECTED (COLOR_BRIGHT  | PAPER_YELLOW | INK_BLACK)
#define COLOR_ERROR (COLOR_BRIGHT  | COLOR_BLINK | PAPER_BLACK | INK_RED)

void UI_ClearLine(int line, int color);
void UI_PixMem(int x, int y, int *bmp, int *col);
void UI_Cls(int col);
void UI_PrintCh(int x, int y, unsigned char c, int col);
void UI_PrintStr(int x, int y, char *str, int col);
void UI_PrintFStr(int x, int y, char *str, int col);
void UI_PrintChBig(int x, int y, unsigned char c, int col);
void UI_PrintStrBig(int x, int y, char *str, int col);
void UI_PrintFStrBig(int x, int y, char *str, int col);

#endif
