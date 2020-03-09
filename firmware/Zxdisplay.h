#ifndef SCREEN_H
#define SCREEN_H

void zxDisplaySetup(unsigned char *ram);
#ifdef __cplusplus
extern "C" {
#endif
void  zxDisplayBorderSet(int i);
void  zxDisplayWriteSerial(int i);
int zxDisplayBorderGet(void);

#ifdef __cplusplus
}
#endif

void zxDisplayScan(void);

void  zxDisplayStartWrite(void);
void  zxDisplayContinueWrite(char*buffer, int counter);
void  zxDisplayStopWrite(void);

void zxDisplayReset(void);
void zxDisplayDisableInterrupt(void);
void zxDisplayEnableInterrupt(void);
void zxDisplaySetIntFrequency(int);


#endif
