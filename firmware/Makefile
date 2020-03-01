ESP_SDK := d:\\arduino_8266nowifi\\portable\\packages\\esp8266\\hardware\\esp8266\\2.4.0
ARDUINO_TMP := .arduino.tmp

CC := xtensa-lx106-elf-gcc
CXX := xtensa-lx106-elf-g++
LD := xtensa-lx106-elf-ld
AR := xtensa-lx106-elf-ar

ESPTOOL := "${ESP_SDK}"/../../../tools/esptool/0.4.12/esptool

INCLUDES += -I ${ESP_SDK}\\tools\\sdk\\include\\
INCLUDES += -I ${ESP_SDK}\\tools\\sdk\\lwip\\include 
INCLUDES += -I ${ESP_SDK}\\tools\\sdk\\libc\\xtensa-lx106-elf\\include\\

INCLUDES += -I ${ESP_SDK}\\cores\\esp8266
INCLUDES += -I ${ESP_SDK}\\cores\\esp8266\\umm_malloc
INCLUDES += -I ${ESP_SDK}\\\variants\\nodemcu
INCLUDES += -I ${ESP_SDK}\\libraries\\SPI

LIBRARY_PATH += -L ${ESP_SDK}\\tools\\sdk\\lib\\
LIBRARY_PATH += -L ${ESP_SDK}\\tools\\sdk\\libc\\xtensa-lx106-elf\\lib\\
LIBRARY_PATH += -L ${ESP_SDK}\\tools\\sdk\\ld\\

INCLUDES += -I lib\\SdFat\\src
INCLUDES += -I lib\\TFT_eSPI
INCLUDES += -I.

DEFINES = -U__STRICT_ANSI__ -D__ets__ -DICACHE_FLASH -DF_CPU=80000000L -DLWIP_OPEN_SRC -DARDUINO=10809 -DARDUINO_ESP8266_NODEMCU -DARDUINO_ARCH_ESP8266 -DARDUINO_BOARD=\"ESP8266_NODEMCU\" -DESP8266 -DESP8266_NOWIFI
OPTIONS = -w  -O2 -g -mtext-section-literals -mlongcalls -falign-functions=4 -ffunction-sections -fdata-sections

#TFT_eSPI configuration (one can change SPI frequency too with -DSPI_FREQUENCY=80000000)
DEFINES += -DUSER_SETUP_LOADED -DILI9341_DRIVER -DTFT_CS=PIN_D1 -DTFT_DC=PIN_D4 -DTFT_RST=-1

CFLAGS += -std=gnu99 -fno-inline-functions -Wpointer-arith
CFLAGS += ${INCLUDES}
CFLAGS += ${DEFINES}
CFLAGS += ${OPTIONS}

CXXFLAGS += ${INCLUDES}
CXXFLAGS += ${DEFINES}
CXXFLAGS += ${OPTIONS}
CXXFLAGS += -Wno-delete-non-virtual-dtor-mlongcalls  -fno-exceptions -fno-rtti -fworking-directory -std=c++11 -x c++

LDFLAGS = -nostdlib --no-check-sections -u call_user_start -u _printf_float -u _scanf_float -static --gc-sections -wrap system_restart_local -wrap spi_flash_read
LDFLAGS += ${LIBRARY_PATH} --start-group -T eagle.app.v6.ld ${ARDUINO_TMP}/arduino.ar -lstdc++ -lm -lc -lmgcc -lloader -lsys

# Project sources
CXX_SRC := ${ARDUINO_TMP}/firmware.cpp SdNavigation.cpp SpiSwitch.cpp Zxdisplay.cpp Zxsound.cpp ShowKeyboard.cpp Z80filedecoder.cpp Zxkeyboard.cpp

# User libraries
CXX_SRC += lib/TFT_eSPI/TFT_eSPI.cpp
CXX_SRC += lib/SdFat/src/FatLib/FatFile.cpp lib/SdFat/src/FatLib/FatFileLFN.cpp lib/SdFat/src/FatLib/FatFileSFN.cpp lib/SdFat/src/FatLib/FatVolume.cpp
CXX_SRC += lib/SdFat/src/SpiDriver/SdSpiESP8266.cpp
CXX_SRC += lib/SdFat/src/SdCard/SdSpiCard.cpp
C_SRC := conDebug.c debug.c z80.c

# Arduino core library
ARDUINO_SRC := abi.cpp core_esp8266_main.cpp core_esp8266_wiring.c core_esp8266_wiring_digital.c core_esp8266_noniso.c core_esp8266_postmortem.c core_esp8266_timer.c core_esp8266_wiring_pwm.c 
ARDUINO_SRC += libc_replacements.c heap.c pgmspace.cpp umm_malloc.c uart.c
ARDUINO_SRC += Esp.cpp FS.cpp HardwareSerial.cpp Print.cpp Schedule.cpp Stream.cpp WString.cpp

# Other Arduino libraries
ARDUINO_INTERNAL_LIBS += SPI.cpp

ARDUINO_SRC_TMP := $(addprefix ${ARDUINO_TMP}/, ${ARDUINO_SRC})
ARDUINO_INTERNAL_LIBS_TMP := $(addprefix ${ARDUINO_TMP}/, ${ARDUINO_INTERNAL_LIBS})

all: quorum8266.bin

Zxrom.h: roms/ZX48.ROM
	xxd -i $< | tail -n +2 | head -n -2 > $@

# To simplify things we do Arduino way and copy required source files to temporary directory

# Generic rules for .c/.cpp files copy

${ARDUINO_TMP}/%.cpp: ${ESP_SDK}/cores/esp8266/%.cpp
	[ -d ${ARDUINO_TMP} ] || mkdir ${ARDUINO_TMP}
	cp $< $@

${ARDUINO_TMP}/%.c: ${ESP_SDK}/cores/esp8266/%.c
	[ -d ${ARDUINO_TMP} ] || mkdir ${ARDUINO_TMP}
	cp $< $@

${ARDUINO_TMP}/firmware.cpp: firmware.ino Zxrom.h
	[ -d ${ARDUINO_TMP} ] || mkdir ${ARDUINO_TMP}
	cp $< $@

# Couple of files are stored in somewhere in Arduino subdirectories. I'm too lazy to implement generic rule for them

${ARDUINO_TMP}/SPI.cpp: ${ESP_SDK}/libraries/SPI/SPI.cpp
	[ -d ${ARDUINO_TMP} ] || mkdir ${ARDUINO_TMP}
	cp $< $@

${ARDUINO_TMP}/umm_malloc.c: ${ESP_SDK}/cores/esp8266/umm_malloc/umm_malloc.c
	[ -d ${ARDUINO_TMP} ] || mkdir ${ARDUINO_TMP}
	cp $< $@

# Build rules

%.cpp.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

%.c.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

# Make arduino.ar archive the same way the IDE does it

${ARDUINO_TMP}/arduino.ar: ${ARDUINO_SRC_TMP:=.o}
	${AR} cru $@ $^

clean:
	rm quorum8266.hex Zxrom.h ${ROMS} || true
	rm -rf ${ARDUINO_TMP}
	find -name '*.o' -delete

quorum8266.elf: ${CXX_SRC:=.o} ${C_SRC:=.o} ${ARDUINO_TMP}/arduino.ar ${ARDUINO_INTERNAL_LIBS_TMP:=.o}
	${LD} -o $@ $^ ${LDFLAGS}

quorum8266.bin: quorum8266.elf
	${ESPTOOL} -eo "${ESP_SDK}/bootloaders/eboot/eboot.elf" -bo $@ -bm qio -bf 40 -bz 4M -bs .text -bp 4096 -ec -eo $< -bs .irom0.text -bs .text -bs .data -bs .rodata -bc -ec

flash: quorum8266.bin
	${ESPTOOL} -cd nodemcu -cb 115200 -cp ${PORT} -ca 0 -cf $<

ROM/ROM_%.h: ROM/%.ROM
	xxd -i $< | tail -n +2 | head -n -2 > $@
