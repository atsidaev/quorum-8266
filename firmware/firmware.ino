/*
   acknowledge of others valuable works:
   Marat Fayzullin for z80 cpu emulation
   https://mikrocontroller.bplaced.net/wordpress/?page_id=756
   https://github.com/uli/Arduino_nowifi
   https://github.com/greiman/SdFat
*/

#include <Arduino.h>
#include "Zxdisplay.h"
#include "Zxsound.h"
#include "Zxkeyboard.h"
#include "GlobalDef.h"
#include "ShowKeyboard.h"
#include "lib/Z80/Z80.h"
#include "z80_operations.h"
#include "Z80filedecoder.h"
#include "SpiSwitch.h"
#include "SdNavigation.h"
#include "Hardware/SerialKeyboard.h"
#include "UI/CommonUI.h"

void process_keyboard_events();

static char ongoingtask = EMUTASK_EMULATOR; //0=emulator,1=file browser,2=display keyboard,3=load demo from rom

unsigned char RAM[RAMSIZE];//48k
unsigned char CACHE[ZXSCREENSIZE]; //used for video backup and file decoding
const unsigned char ROM[ROMSIZE] PROGMEM = {
#include "zxrom.h" //original rom
};

Z80 state;//emulator status
int z80DelayCycle = 1;

char zxInterruptPending = 0;//enabled by interrupt routine
int timerfreq = 50; //default
unsigned char soundenabled = 1; //default sound on

void zxEnableInterrupt();
void zxDisableInterrupt();

void ICACHE_FLASH_ATTR stampabinario(unsigned char c);

int ticks = 0, last_ticks = 0;
int int_count = 0;

volatile uint16_t timer_int = 0;

void ICACHE_FLASH_ATTR setup() {
#ifdef DEBUG_PRINT
  Serial.begin(115200);
#endif.

  spiSwitchSetup();

  sdNavigationSetup();
  zxDisplaySetup(RAM);
  zxKeyboardSetup();
  zxSoundSetup();

  zxEnableInterrupt();

  ResetZ80(&state);
}

/////////////////////////////////////

// SPI_FREQUENCY - frequency of screen's bus

#define TIMER_DIVISOR 12500 // 100000 interrupts per second
int TIMER_50HZ = 2000; //480=50hz
//every 480 interrupt, timer keyboard is read instead of writing display

volatile int zxDisplay_timer_counter_50hz = 0;

//called 100000 times per second
void zxDisplay_timer1_ISR (void) {
  timer_int++;
}

void zxEnableInterrupt()
{
	timer1_disable();
	timer1_attachInterrupt(zxDisplay_timer1_ISR);
	timer1_isr_init();

	/*
	160 Mhz ESP, 3.5 MHz Z80, 50 Hz screen, 2400 screen updates per frame (120000 per second)

	TIM_DIV16 gives us 10 Mhz of base timer freq
	TIMER_DIVISOR = 83, which gives us 120481 executions of interrupt per second

	Each frame: 32 pixels, then 32 tacts CPU
	*/

	timer1_enable(TIM_DIV265, TIM_EDGE, TIM_LOOP);//80mhz/16=5mhz
	// 2400 blocks of 32 pixel to redraw the screen
	//to get 10hz-->24000 interrupt-->divisor 208
	timer1_write(TIMER_DIVISOR);
}

int ICACHE_FLASH_ATTR showJoystick()
{
  static char cStep = 0;

  switch (cStep)
  { case 0:
      pJoyKeyVal[0] = pJoyKeyVal[1] = pJoyKeyVal[2] = pJoyKeyVal[3] = pJoyKeyVal[4] = pJoyKeyVal[5] = 0xff;
      cStep++;
      UI_Cls(COLOR_TEXT);
      waitforclearkeyb();
      return 0;
    case 1:
      cStep++;
      UI_PrintFStrBig(0, 0, F("Press UP   "), COLOR_TEXT);
      getKeyb(&pJoyKeyAdd[0], &pJoyKeyVal[0]);
      waitforclearkeyb();
      return 0;
    case 2:
      cStep++;
      UI_PrintFStrBig(0, 0, F("Press DOWN "), COLOR_TEXT);
      getKeyb(&pJoyKeyAdd[1], &pJoyKeyVal[1]);
      waitforclearkeyb();
      return 0;
    case 3:
      cStep++;
      UI_PrintFStrBig(0, 0, F("Press LEFT "), COLOR_TEXT);
      getKeyb(&pJoyKeyAdd[2], &pJoyKeyVal[2]);
      waitforclearkeyb();
      return 0;
    case 4:
      cStep++;
      UI_PrintFStrBig(0, 0, F("Press RIGHT"), COLOR_TEXT);
      getKeyb(&pJoyKeyAdd[3], &pJoyKeyVal[3]);
      waitforclearkeyb();
      return 0;
    case 5:
      cStep++;
      UI_PrintFStrBig(0, 0, F("Press FIRE1"), COLOR_TEXT);
      getKeyb(&pJoyKeyAdd[4], &pJoyKeyVal[4]);
      waitforclearkeyb();
      return 0;
    case 6:
      cStep = 0;
      UI_PrintFStrBig(0, 0, F("Press FIRE2"), COLOR_TEXT);
      getKeyb(&pJoyKeyAdd[5], &pJoyKeyVal[5]);
      waitforclearkeyb();
      return -1;
  }
}

//return 0 to do nothing
//1..n to select an option
//-1 to return to program
int ICACHE_FLASH_ATTR showMenu()
{
  static boolean bdisplayedmenu = false;

  if (!bdisplayedmenu)
  {
    bdisplayedmenu = true;
    UI_Cls(COLOR_TEXT);
    UI_PrintFStrBig(0, 22, F("BRK to exit"), COLOR_TEXT);
    UI_PrintFStrBig(0, 0, F("1 SD list"), COLOR_TEXT);
    UI_PrintFStrBig(0, 2, F("2 EEPROM list"), COLOR_TEXT);
    UI_PrintFStrBig(0, 4, F("3 ZX keyboard"), COLOR_TEXT);
    UI_PrintFStrBig(0, 6, F("4 CPU"), COLOR_TEXT);
    switch (z80DelayCycle)
    { case CPUDELAYSLOWEST:// 500
        UI_PrintFStrBig(12, 6, F("SLOWEST"), COLOR_TEXT);
        break;
      case CPUDELAYSLOWER:// 1000
        UI_PrintFStrBig(12, 6, F("SLOWER "), COLOR_TEXT);
        break;
      case CPUDELAYNORMAL:// 5000
        UI_PrintFStrBig(12, 6, F("NORMAL "), COLOR_TEXT);
        break;
      case CPUDELAYFASTER:// 5000
        UI_PrintFStrBig(12, 6, F("FASTER "), COLOR_TEXT);
        break;
      case CPUDELAYFASTEST:// 50000
        UI_PrintFStrBig(12, 6, F("FASTEST"), COLOR_TEXT);
        break;
    }

    UI_PrintFStrBig(0, 8, F("5 Timer ..Hz"), COLOR_TEXT);
    UI_PrintChBig(16, 8, '0' + (timerfreq / 10), COLOR_TEXT);
    UI_PrintChBig(18, 8, '0' + (timerfreq % 10), COLOR_TEXT);

    UI_PrintFStrBig(0, 10, F("6 Sound"), COLOR_TEXT);
    if (soundenabled)
    {
      UI_PrintFStrBig(16, 10, F("ON"), COLOR_TEXT);
    }
    else
    {
      UI_PrintFStrBig(16, 10, F("OFF"), COLOR_TEXT);
    }
    UI_PrintFStrBig(0, 12, F("7 Joystik setup"), COLOR_TEXT);

    UI_PrintFStrBig(0, 14, F("R Reset"), COLOR_TEXT);
    UI_PrintFStrBig(0, 16, F("S Save Z80 file"), COLOR_TEXT);

    //before returning wait for an empty keyb buffer
    waitforclearkeyb();
    return 0;
  }

  if (   checkKeybBreak() || checkKeyBit(&(KEY[4]),  BUTTON_0)
         || (KEMPSTONJOYSTICK && BUTTON_LEFT) ) //'0' key or left
  {
    bdisplayedmenu = false;
    return -1;//back to old program
  }



  if (   checkKeyBit(&(KEY[2]),  BUTTON_R) ) //'R'
  {
    bdisplayedmenu = false;
    ResetZ80(&state);
    return -1;//back to old program
  }

  if (   checkKeyBit(&(KEY[1]),  BUTTON_S) ) //'S'
  {
    bdisplayedmenu = false;
    char filename[32];
    if (sdNavigationGetFileName(filename)==0)
    {
      strcat(filename, ".z80");
      //before saving video restore is required. 
      memcpy( RAM, CACHE, ZXSCREENSIZE);
      sdNavigationFileSave(filename);
      //cache video again before returning
      memcpy( CACHE, RAM, ZXSCREENSIZE);
    }

    return -1;//back to old program
  }

  if (checkKeyBit(&(KEY[4]),  BUTTON_6) ) //'6'
  {
    soundenabled ^= 1;
    bdisplayedmenu = false;
    return 0;
  }




  if (checkKeyBit(&(KEY[3]),  BUTTON_1) ) //'1'
  {
    bdisplayedmenu = false;
    return EMUTASK_SD;//sd upload
  }
  if (checkKeyBit(&(KEY[3]),  BUTTON_2) ) //'2'
  {
    bdisplayedmenu = false;
    return EMUTASK_EPROM;//eprom upload
  }
  if (checkKeyBit(&(KEY[3]),  BUTTON_3) ) //'3'
  {
    bdisplayedmenu = false;
    return EMUTASK_KEYB;//keyboard
  }

  if (checkKeyBit(&(KEY[4]),  BUTTON_7) ) //'3'
  {
    bdisplayedmenu = false;
    return EMUTASK_JOY;//setup joystick
  }




  if (checkKeyBit(&(KEY[3]),  BUTTON_4) ) //'4'
  {
    bdisplayedmenu = false;

    switch (z80DelayCycle) {
      case CPUDELAYSLOWEST:
        z80DelayCycle  = CPUDELAYSLOWER;
        break;
      case CPUDELAYSLOWER:
        z80DelayCycle  = CPUDELAYNORMAL;
        break;
      case CPUDELAYNORMAL:
        z80DelayCycle  = CPUDELAYFASTER;
        break;
      case CPUDELAYFASTER:
        z80DelayCycle  = CPUDELAYFASTEST;
        break;
      case CPUDELAYFASTEST:
        z80DelayCycle  = CPUDELAYSLOWEST;
        break;
    }


    return 0;
  }

  if (checkKeyBit(&(KEY[3]),  BUTTON_5) ) //'4'
  {
    switch (timerfreq)
    {
      case 25:
        timerfreq = 38;
        break;
      case 38:
        timerfreq = 50;
        break;
      case 50:
        timerfreq = 62;
        break;
      case 62:
        timerfreq = 75;
        break;
      case 75:
        timerfreq = 25;
        break;
    }
    bdisplayedmenu = false;
    return 0;
  }

  return 0;
}

void loop() {
	while (true)
	{
		const int ticks_per_iteration = 32;
		for (uint16_t i = 0; i < (76800 / ticks_per_iteration); i++)
		{
			for (uint8_t k = 0; k < ticks_per_iteration / 32; k++)
				zxDisplayScan();//show 32 pixel at a time

			ExecZ80(&state, ticks_per_iteration);
			ticks += ticks_per_iteration;
		}

		// Detect skipped 50Hz interrupts - if there are any, we should do frame skipping OR speed up the processing
		if (timer_int > 1)
		{
			Serial.println(timer_int);
		}

		IntZ80(&state, INT_IRQ);

		serial_keyboard_read();

		// Check if we need to change state of emulator because of pressed keys
		// process_keyboard_events();

		while (timer_int == 0) {}
		timer_int = 0;
	}
}

void process_keyboard_events()
{
	switch (ongoingtask)
	{
		case EMUTASK_EMULATOR: //emulator
			if (KEMPSTONJOYSTICK & BUTTON_ESC) //'ESC special key
			{
				DEBUG_PRINTLN("ESC");
				ongoingtask = EMUTASK_MENU;//keyb display
				memcpy(CACHE, RAM, ZXSCREENSIZE);
				break;
			}
		case EMUTASK_MENU:
		{
			int iReturn = showMenu();
			if (iReturn < 0)
			{
				//back to old program
				ongoingtask = EMUTASK_EMULATOR;
				memcpy(RAM, CACHE, ZXSCREENSIZE);//restore screen
				waitforclearkeyb();
			}
			if (iReturn > 0)
			{
				ongoingtask = iReturn;
			}
		}
		break;

		case EMUTASK_JOY:
			if (showJoystick() < 0)
			{
				//back to old program
				ongoingtask = EMUTASK_EMULATOR;
				memcpy(RAM, CACHE, ZXSCREENSIZE);//restore screen
				waitforclearkeyb();
			}
			break;

		case EMUTASK_KEYB:
			if (showKeyboard() < 0)
			{
				//back to old program
				ongoingtask = EMUTASK_EMULATOR;
				memcpy(RAM, CACHE, ZXSCREENSIZE);//restore screen
				waitforclearkeyb();
			}
			break;

		case EMUTASK_SD://file browser
			switch (sdNavigation(false))
			{
				case -1://back to old program
					ongoingtask = EMUTASK_EMULATOR;
					memcpy(RAM, CACHE, ZXSCREENSIZE);
					waitforclearkeyb();
					break;
				case 2://jump to new proGRAM
					ongoingtask = EMUTASK_EMULATOR;
					break;
			}
			break;
		case EMUTASK_EPROM://eprom browser
			switch (sdNavigation(true))
			{
				case -1://back to old program
					ongoingtask = EMUTASK_EMULATOR;
					memcpy(RAM, CACHE, ZXSCREENSIZE);
					waitforclearkeyb();
					break;
				case 2://jump to new proGRAM
					ongoingtask = EMUTASK_EMULATOR;
					break;
			}
			break;

	}
}

void ICACHE_FLASH_ATTR stampabinario(unsigned char c)
{
  DEBUG_PRINT(c & 128 ? "1" : "0");
  DEBUG_PRINT(c & 64 ? "1" : "0");
  DEBUG_PRINT(c & 32 ? "1" : "0");
  DEBUG_PRINT(c & 16 ? "1" : "0");
  DEBUG_PRINT(c & 8 ? "1" : "0");
  DEBUG_PRINT(c & 4 ? "1" : "0");
  DEBUG_PRINT(c & 2 ? "1" : "0");
  DEBUG_PRINT(c & 1 ? "1" : "0");
}
