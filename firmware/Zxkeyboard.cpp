/*
   spi run at 3mhz max to read 50 bits. display run at 20mhz to transfer 512 bits every 32 pixel, so keyboard reading fits the same
   time of 32 pixels transfer


*/


#include <TFT_eSPI.h> // Hardware-specific library
//#include <SPI.h>
#include <Arduino.h>
#include "Zxdisplay.h"
#include "GlobalDef.h"
#include "SpiSwitch.h"
#include "Hardware/Keyboard.h"
#include "z80_operations.h"

boolean keyboardCheckAny(unsigned char *pBuffer, int len);
boolean keyboardCheckKeyStatusRowCol(unsigned char *pBuffer, int iRow, int iCol, unsigned char  * off, unsigned char * mask);
boolean keyboardCheckKeyStatus(unsigned char *pBuffer, unsigned char off, unsigned char mask);

extern unsigned char *pJoyKeyAdd[6] ;
extern unsigned char  pJoyKeyVal[6] ;
//up down left right f1 f2

void RefreshScreen();

/*
  1 2 3 4 5     f7fe   6 7 8 9 0       effe
  Q W E R T     fbfe   y U I O P       defe
  A S D F G     fdfe   H J K L e       bffe
  S Z X C V     fefe   B N M S s       7ffe

  //some keys are backward mapped compared to physical location
  bit
  0 1 2 3 4

  1 2 3 4 5
  Q W E R T
  A S D F G
  S Z X C V

  bit
  4 3 2 1 0

  6 7 8 9 0
  y U I O P
  H J K L e
  B N M S s



  Bits are set to 0 for any key that is pressed and 1 for any key that is not pressed. Multiple key presses can be read simultaneously.
*/

//00012345-->00054321
unsigned char zxKeyboardBitRotarion[] = {
  B00000000,
  B00010000,
  B00001000,
  B00011000,
  B00000100,
  B00010100,
  B00001100,
  B00011100,

  B00000010,
  B00010010,
  B00001010,
  B00011010,
  B00000110,
  B00010110,
  B00001110,
  B00011110,

  B00000001,
  B00010001,
  B00001001,
  B00011001,
  B00000101,
  B00010101,
  B00001101,
  B00011101,


  B00000011,
  B00010011,
  B00001011,
  B00011011,
  B00000111,
  B00010111,
  B00001111,
  B00011111
};








//bits read from spi are remapped to zx port using a buffer
void zxKeyboardStopRead(void)
{
  while (SPI1CMD & SPIBUSY);
  SPI.setFrequency(SPI_SPEED_TFT);//restore display spi speed

  //ESP8266_REG(addr) *((volatile uint32_t *)(0x60000000+(addr)))
  //#define SPI1W0     ESP8266_REG(0x140)
  volatile uint8_t *p = (volatile uint8_t *)(0x60000000 + 0x140);

#ifdef ZXKEYBOARDFILLBUFFER
  //bits received from spi
  //R 000000000011111111112222222222333333333344444444445555555555 4017 ROW SELECTION
  //C012345678901234567890123456789012345678901234567890123456789 4017 COLUMN SELECTION

  // 0000000011111111222222223333333344444444555555556666666677777777
  //          SzxcvbnmSSasdfghjklEqwertyuiop1234567890EFFrldu
  //                                                  E12RLDU JOYSTICK + ESC BUTTON
  //  bit
  //  4 3 2 1 0
  //  6 7 8 9 0
  KEY[4] =  ~(((*(p + 5) << 1) & (16 + 8 + 4 + 2)) + ((*(p + 6) >> 7) & 1));
  //  y U I O P
  KEY[5] =   ~(((*(p + 4) >> 1) & (31)));
  //  H J K L e
  KEY[6] =   ~(((*(p + 3) >> 3) & (31)));
  //  B N M S s
  KEY[7] =   ~(((*(p + 1) << 3) & (16 + 8)) + ((*(p + 2) >> 5) & 7));
  //  bit
  //  0 1 2 3 4
  //  1 2 3 4 5
  KEY[3] =   ~zxKeyboardBitRotarion[((*(p + 4) << 4) & 16) + ((*(p + 5) >> 4) & 15)];
  //  Q W E R T
  KEY[2] =   ~zxKeyboardBitRotarion[((*(p + 3) << 2) & (16 + 8 + 4)) + ((*(p + 4) >> 6) & (1 + 2))];
  //  A S D F G
  KEY[1] =   ~zxKeyboardBitRotarion[((*(p + 2)) & 31)];
  //  S Z X C V
  KEY[0] =   ~zxKeyboardBitRotarion[((*(p + 1) >> 2) & 31)];


  //?^JJJJJJ--> n/a ESC F1 F2 R L D U
  //#define BUTTON_LEFT 4
  //#define BUTTON_RIGHT 8
  //#define BUTTON_UP 1
  //#define BUTTON_DONW 2
  //#define BUTTON_FIRE 32
  //#define BUTTON_FIRE2 16
  //#define BUTTON_ESC 64
  KEMPSTONJOYSTICK = 127 & ( *(p + 6));

  //extern unsigned char *pJoyKeyAdd[6] ;
  //extern unsigned char  pJoyKeyVal[6] ;
  //up down left right f1 f2

  if (KEMPSTONJOYSTICK & BUTTON_UP && pJoyKeyVal[0] != 0xff)
  {
    *pJoyKeyAdd[0] &= pJoyKeyVal[0];
  }
  if (KEMPSTONJOYSTICK & BUTTON_DOWN && pJoyKeyVal[1] != 0xff)
  {
    *pJoyKeyAdd[1] &= pJoyKeyVal[1];
  }
  if (KEMPSTONJOYSTICK & BUTTON_LEFT && pJoyKeyVal[2] != 0xff)
  {
    *pJoyKeyAdd[2] &= pJoyKeyVal[2];
  }
  if (KEMPSTONJOYSTICK & BUTTON_RIGHT && pJoyKeyVal[3] != 0xff)
  {
    *pJoyKeyAdd[3] &= pJoyKeyVal[3];
  }
  if (KEMPSTONJOYSTICK & BUTTON_FIRE && pJoyKeyVal[4] != 0xff)
  {
    *pJoyKeyAdd[4] &= pJoyKeyVal[4];
  }
  if (KEMPSTONJOYSTICK & BUTTON_FIRE2 && pJoyKeyVal[5] != 0xff)
  {
    *pJoyKeyAdd[5] &= pJoyKeyVal[5];
  }


#endif
}



//start spi reading for 50 bits. 10 are discarded, 40 are the actual keys
void zxKeyboardStartRead(void)
{
  while (SPI1CMD & SPIBUSY);// return false; //spi not free

  spiSwitchSet(KEYB_CS);

  SPI.beginTransaction(SPISettings(
                         SPI_SPEED_KEYB,//keyboard read frequency
                         MSBFIRST,
                         SPI_MODE0));//esp8266 only support mode 0


  int bitcount = 56 - 1;//10+40keys
  uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));

  SPI1U1 = (SPI1U1 & mask) |
           (bitcount << SPILMOSI) |
           (bitcount << SPILMISO);

  SPI1CMD |= SPIBUSY;

}




void ICACHE_FLASH_ATTR zxKeyboardSetup()
{
  //spi pins enabled in SpiSwitch
}





boolean ICACHE_FLASH_ATTR checkKeyBit( unsigned char *KEY, unsigned char button)
{
  if (!((*KEY) & button) ) // key pressed
  {
    *KEY =  0xff;
    delay(100);
    return true;//back to old program
  }
  return false;
}


//is there a pressed key or joystik? then wait
void ICACHE_FLASH_ATTR waitforclearkeyb()
{
  for (;;)
  {
    RefreshScreen();
    
    if (
      KEY[0] == 0xff &&
      KEY[1] == 0xff &&
      KEY[2] == 0xff &&
      KEY[3] == 0xff &&
      KEY[4] == 0xff &&
      KEY[5] == 0xff &&
      KEY[6] == 0xff &&
      KEY[7] == 0xff &&
      KEMPSTONJOYSTICK == 0) return;
  }
}



//get the pressed key from buffer
void ICACHE_FLASH_ATTR getKeyb(unsigned char **p, unsigned char *v)
{
  for (;;)
  {
    RefreshScreen();
    
    for (int i = 0; i < 8; i++)
    {
      if (KEY[i] != 0xff)
      {
        *p = &KEY[i];
        *v = KEY[i];
        return;
      }
    }
  }
}


boolean ICACHE_FLASH_ATTR checkKeybBreak()
{
  if ( checkKeyBit(&(KEY[0]),  BUTTON_CS) && checkKeyBit(&(KEY[7]),  BUTTON_SP) )
  {
    return true;
  }

  return false;
}


char ICACHE_FLASH_ATTR getPressedCharacter()
{

  // \a   for unexisting characters
  // \b  for delete
  // \n  for enter
  //regular key
  //byte 0..3          //byte 4..7
  unsigned char *cLow = (unsigned char *)"\azxcvasdfgqwert1234509876poiuy\nlkjh \amnb";

  //symbol shift
  //byte 0..3          //byte 4..7
  unsigned char *cShift = (unsigned char *)"\a:\x60?/~|\\{}\a\a\a<>!@#$%_)('&\";\a[]\a=+-^\a\a.,*";
  //                                             \x60=£ special character. not ascii
  //caps shift
  //byte 0..3          //byte 4..7
  unsigned char *cUpper = (unsigned char *)"\aZXCVASDFGQWERT\a\a\a\a\a\b\a\a\a\aPOIUY\aLKJH\a\a\a\a\a";






  char c = 0;
  int ibit = 0;
  int offset = 0;
  int i = 0;
  boolean bCS = false;
  boolean bSS = false;


  if ( checkKeyBit(&(KEY[0]),  BUTTON_CS) )
  {
    bCS = true; //caps shift
  }
  else
  {
    //no cs? maybe ss
    if ( checkKeyBit(&(KEY[7]),  BUTTON_SS) )
    {
      bSS = true; //caps shift
    }

  }

  //sweep along bits to find a single hit
  for (i = 0; i < 8; i++)
  {
    offset = 0;
    ibit = 1;
    if (KEY[i] == 0xff) continue;
    //find the 0 bit
    for (; ibit <= 16; ) {
      if (!(ibit & KEY[i]))
      {
        //is this a real key?
        int coff = i * 5 + offset;
        unsigned char pressed = cLow[coff];

        if (bCS == true) pressed = cUpper[coff];
        if (bSS == true) pressed = cShift[coff];
        if (pressed != '\a')
        {

          //          DEBUG_PRINTLN("Key found");
          //          DEBUG_PRINTLN(coff);
          //          DEBUG_PRINTLN(i);
          //          DEBUG_PRINTLN(offset);
          //          DEBUG_PRINTLN(pressed);

          c = pressed;
          break;
        }
      }
      ibit <<= 1;
      offset++;
    }
    if (c != 0) break;

  }



  if (c)
  { //wait for release of the pressed key
    for (;;)
    {
      RefreshScreen();
      // DEBUG_PRINTLN("wait");
      // DEBUG_PRINTLN((volatile unsigned char)KEY[i]);
      // DEBUG_PRINTLN(i);
      // DEBUG_PRINTLN(ibit);
      if (ibit & (volatile unsigned char)KEY[i]) break;
    }
  }




  return c;
}
