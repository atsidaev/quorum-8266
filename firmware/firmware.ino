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
#include "lib/Z80/Z80.h"
#include "z80_operations.h"
#include "SpiSwitch.h"
#include "SdNavigation.h"
#include "Hardware/SerialKeyboard.h"
#include "UI/MainMenu.h"

void process_keyboard_events();

unsigned char RAM[RAMSIZE];//48k
const uint32_t ROM[ROMSIZE] PROGMEM = {
#include "zxrom.h" //original rom
};

Z80 state;//emulator status

char zxInterruptPending = 0;//enabled by interrupt routine

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
		if (KEMPSTONJOYSTICK & BUTTON_ESC)
			DrawMenu();

		while (timer_int == 0) { }
		timer_int = 0;
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
