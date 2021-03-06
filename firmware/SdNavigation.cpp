//browse sd card or demo app storedin eeprom
#include <SPI.h>
#include "SdFat.h"
#include "FreeStack.h"
#include "Zxdisplay.h"
#include "Zxsound.h"
#include "Z80filedecoder.h"
#include "Zxkeyboard.h"
#include "GlobalDef.h"
#include "SpiSwitch.h"
#include "SdNavigation.h"
#include "Hardware/Keyboard.h"

#include "UI/CommonUI.h"

//to add/change demos in rom:
//-change z80file_demo.h content
//-change iromfilesize
//-change promfilename. leave the last 0 there
#include "Z80file_demo.h"
#ifdef NOGAMESROM
const unsigned char * pRomFile[] = {};
int iRomFileSize[] = {};
char * pRomFileName[] = {0};
#else
const unsigned char * pRomFile[] = {z80file_bubblefrenzy, z80file_fistrofighter, z80file_invasivespecies};
int iRomFileSize[] = {sizeof(z80file_bubblefrenzy), sizeof(z80file_fistrofighter), sizeof(z80file_invasivespecies)};
char * pRomFileName[] = {F("Bubble frenzy"), F("Fist RO Fighter"), F("Invasive species"), 0};
#endif


extern void waitforclearkeyb(void);
void RefreshScreen();

#define LISTLEN 11

int sdNavigationReadFiles = -1; //-1 if nothing available
int sdNavigationIndex = 0;
int sdNavigationCursor = 0;

//stop display scan
void ICACHE_FLASH_ATTR sdNavigationLockSPI()
{
  // zxDisableInterrupt();
  delay(100);//enough to consume an interrupt
  spiSwitchSet(SD_CS);
}


//after sd card usage restore spi for display
void ICACHE_FLASH_ATTR sdNavigationUnlockSPI()
{
  spiSwitchSet(TFT_CS);
  SPI.setFrequency(SPI_SPEED_TFT);//restore display spi speed
  //zxEnableInterrupt();
}

/*perform a command callbackprocess on the files of the directrory
   from fromindex file. up to maxfiles processed. callbackfilter
   remove unwanted files form list.
   example: print on screen the found files ending with .z80
   callbackprocess print the file according to its index
   callbackfilter check the filename for .z80 extension

   if the command fails return -1. for example in case of removed sd
   or not formatted sd

   the command return the number of found files. 0 at the end

   no sd initialization is required before
   remember to unlock spi before return
*/
int ICACHE_FLASH_ATTR sdNavigationFileProcess(int fromindex, int maxfiles,
    boolean(*callbackFilter)(SdFile *file, int indx),
    void(*callbackProcess)(SdFile *file, int indx)) {


  DEBUG_PRINTLN("listfiles");
  DEBUG_PRINTLN(fromindex);
  DEBUG_PRINTLN(maxfiles);

  sdNavigationLockSPI();

  SdFat sd;
  SdFile file;
  SdFile dirFile;
  int foundfiles = 0;
  int index = 0;



  if (!sd.begin(SD_CS, SPISettings(
                  SPI_SPEED_SD,
                  MSBFIRST,
                  SPI_MODE0))) {
    sdNavigationUnlockSPI();
    return -1;
  }

  // List files in root directory. always start from the first
  if (!dirFile.open("/", O_RDONLY)) {
    sdNavigationUnlockSPI();
    return -1;
  }

  while (file.openNext(&dirFile, O_RDONLY)) {

    if (foundfiles == maxfiles)
    {
      sdNavigationUnlockSPI();
      return foundfiles;
    }

    if (callbackFilter != NULL)
    {
      if (!(*callbackFilter)(&file, index))
      {
        file.close();
        continue;
      }
    }


    // Skip directories and hidden files.
    if (file.isSubDir() || file.isHidden()) {
      //do nothing
      file.close();
      continue;
    }

    //found a file
    if (index >= fromindex)
    {
      //DEBUG_PRINT(index);
      //DEBUG_PRINT(' ' );
      //file.printName(&Serial);
      //DEBUG_PRINTLN  ();
      if (callbackProcess != NULL)  (*callbackProcess)(&file, index);
      foundfiles++;
    }
    index++;

    file.close();
  }

  sdNavigationUnlockSPI();
  return foundfiles;//no sd error
}





int ICACHE_FLASH_ATTR sdNavigationProcessFromEeprom(int fromindex, int maxfiles,
    void(*callbackProcess)( int indx)) {

  DEBUG_PRINTLN("eepromfiles");
  DEBUG_PRINTLN(fromindex);
  DEBUG_PRINTLN(maxfiles);

  int foundfiles = 0;
  int index = 0;




  for (; pRomFileName[index];) {

    if (foundfiles == maxfiles)
    {
      return foundfiles;
    }


    //found a file
    if (index >= fromindex)
    {
      //DEBUG_PRINT(index);
      //DEBUG_PRINT(' ' );
      //file.printName(&Serial);
      //DEBUG_PRINTLN  ();
      if (callbackProcess != NULL)  (*callbackProcess)(index);
      foundfiles++;
    }
    index++;

  }

  return foundfiles;//no sd error
}







//write index SPACE and file name.
//color according to current cursor position
void ICACHE_FLASH_ATTR sdNavigationCallbackPrint(SdFile *file, int indx)
{
  char str[33];
  int y = 2 * (indx % LISTLEN);

  UI_ClearLine(y, COLOR_FILE);
  UI_ClearLine(y + 1, COLOR_FILE);

  sdNavigationPrintNumberBig(indx, COLOR_FILE);

  file->getName(str, 14);
  UI_PrintStrBig(6, y, str, COLOR_FILE);

  //  int filesize = file->fileSize();
  //  sprintf(str, "%6d", filesize);
  //  UI_PrintStrBig(26, y, str, COLOR_FILE);

  DEBUG_PRINTLN(str);

}

void ICACHE_FLASH_ATTR sdNavigationCallbackPrintFromEeprom(int indx)
{
  char str[33];
  int y = 2 * (indx % LISTLEN);

  UI_ClearLine(y, COLOR_FILE);
  UI_ClearLine(y + 1, COLOR_FILE);

  sdNavigationPrintNumberBig(indx, COLOR_FILE);
  memcpy_P(str, pRomFileName[indx], 13);
  str[13] = 0;
  UI_PrintStrBig(6, y, str, COLOR_FILE);
}

boolean ICACHE_FLASH_ATTR sdNavigationCallbackFilter(SdFile *file, int indx)
{
  char nome[128];
  int len;

  file->getName(nome, 128);
  len = strlen(nome);
  if (len < 4) return false;
  //name.z80 or nome.Z80
  //
  len -= 4;
  if (strcmp(nome + len, ".z80") == 0) return true;
  if (strcmp(nome + len, ".Z80") == 0) return true;
  return false;
}




void ICACHE_FLASH_ATTR sdNavigationCallbackLoad(SdFile *file, int indx)
{
  z80FileLoad(file);
}

void ICACHE_FLASH_ATTR sdNavigationCallbackLoadFromEeprom(int indx)
{

  z80FileLoadFromEeprom(pRomFile[indx], iRomFileSize[indx]);

}









void ICACHE_FLASH_ATTR sdNavigationPrintNumber(int indx, int color)
{
  char str[4];
  sprintf(str, "%3d", indx + 1);
  UI_PrintStr(0, indx % LISTLEN, str, color);
}

void ICACHE_FLASH_ATTR sdNavigationPrintNumberBig(int indx, int color)
{
  char str[3];
  sprintf(str, "%2d", indx + 1);
  UI_PrintStrBig(0, 2 * (indx % LISTLEN), str, color);
}

void ICACHE_FLASH_ATTR sdNavigationReset()
{
  sdNavigationCursor = 0;
  sdNavigationIndex = 0;
  sdNavigationReadFiles = -1;

}



int ICACHE_FLASH_ATTR sdNavigationList(int fromindex, int len, boolean fromEeprom)
{
  static unsigned long ulNextCheck = 0;
  unsigned long ulNow = millis();
  int foundfiles ;

  if (ulNextCheck && ulNow < ulNextCheck) return -1; //timeout before next check
  if (fromEeprom)
  {
    foundfiles = sdNavigationProcessFromEeprom(fromindex, len , &sdNavigationCallbackPrintFromEeprom);
  }
  else {
    foundfiles = sdNavigationFileProcess(fromindex, len , &sdNavigationCallbackFilter, &sdNavigationCallbackPrint);
  }
  if ( foundfiles   < 0)
  {
    ulNextCheck = ulNow + 1000;

    sdNavigationReset();
    UI_Cls(INK_BLACK);
    UI_PrintFStrBig(0, 22, F("SD not available"), COLOR_ERROR);

    return (-1);
  }

  ulNextCheck = 0; //no timeout
  
  //update video
  //
  //clear remaining lines
  //sdNavigationReadFiles = foundfiles;
  if (foundfiles)
  {
    for (int i = foundfiles; i < LISTLEN; i++)
    {
      UI_ClearLine(i * 2, COLOR_FILE);
      UI_ClearLine(i * 2 + 1, COLOR_FILE);
    }
  }

  UI_PrintFStrBig(0, 22, F("Q/A/BRK/ENT/joy "), COLOR_TEXT);


  return foundfiles;
}


int ICACHE_FLASH_ATTR sdNavigation(boolean fromEeprom)
{
  static boolean bdisplayed = false;

  if (sdNavigationReadFiles == -1) //initialize list
  {
    unsigned long int lastcheck = 0;
    unsigned long now = millis();
    if (now - lastcheck < 1000) return -1;
    lastcheck = now;

    sdNavigationReset();
    int foundfiles = sdNavigationList(sdNavigationIndex, LISTLEN, fromEeprom);

    sdNavigationReadFiles = foundfiles;
    if (foundfiles > 0) sdNavigationPrintNumberBig(0, COLOR_FILE_SELECTED);

    if (!bdisplayed)
    {
      bdisplayed = true;
      waitforclearkeyb();
    }
  }

  //check up/down/enter

  //
  //  sdNavigationCursor = 0;
  //  sdNavigationIndex = 0;
  //  sdNavigationReadFiles = -1;

  if (sdNavigationReadFiles > 0)
  {
    if (!(KEY[1] & BUTTON_A) ||  (KEMPSTONJOYSTICK & BUTTON_DOWN)) //'a' key=DOWN
    {
      waitforclearkeyb();

      if ( (sdNavigationCursor + 1) == (sdNavigationIndex + sdNavigationReadFiles)) //goto next page
      {
        // DEBUG_PRINTLN("DOWNPAGE");
        int foundfiles = sdNavigationList(sdNavigationCursor + 1 , LISTLEN, fromEeprom);
        DEBUG_PRINTLN("down");
        DEBUG_PRINTLN(foundfiles);
        if (foundfiles <= 0)//page empty!!! go back...
        {
          delay(100);
          return (0); //no sd or no more files
        }
        //goto next page with cursor on the first line
        sdNavigationCursor++;
        sdNavigationIndex = sdNavigationCursor ;
        sdNavigationReadFiles = foundfiles;
        sdNavigationPrintNumberBig(sdNavigationCursor, COLOR_FILE_SELECTED);
        delay(100);
        return (0);
      }
      //go down a row
      //DEBUG_PRINTLN("DOWNROW");
      sdNavigationPrintNumberBig(sdNavigationCursor, COLOR_FILE);
      sdNavigationCursor++;
      sdNavigationPrintNumberBig(sdNavigationCursor, COLOR_FILE_SELECTED);
      delay(100);
      return (0);

    }
    if (!(KEY[2] & BUTTON_Q) ||  (KEMPSTONJOYSTICK & BUTTON_UP)) //'q' key=UP
    {
      waitforclearkeyb();

      if (sdNavigationCursor == 0) return (0);

      if ((sdNavigationCursor % LISTLEN) == 0) //return to previous page
      {
        int foundfiles = sdNavigationList(sdNavigationIndex - LISTLEN , LISTLEN, fromEeprom);
        if (foundfiles <= 0)//page empty!!! go back...
        {
          delay(100);
          return (0); //no sd or no more files
        }
        //goto prev page with cursor on the first line
        sdNavigationIndex -= LISTLEN;
        sdNavigationCursor--;
        sdNavigationReadFiles = foundfiles;
        sdNavigationPrintNumberBig(sdNavigationCursor, COLOR_FILE_SELECTED);
        delay(100);
        return (0);
      }

      sdNavigationPrintNumberBig(sdNavigationCursor, COLOR_FILE);
      sdNavigationCursor--;
      sdNavigationPrintNumberBig(sdNavigationCursor, COLOR_FILE_SELECTED);
      delay(100);
      return (0);


    }
    if (!(KEY[6] & BUTTON_EN) ||  (KEMPSTONJOYSTICK & BUTTON_FIRE)) //'enter' key  or button
    {
      waitforclearkeyb();
      //scan the root for the selected file and load it
      if (fromEeprom == false)
      {
        sdNavigationFileProcess(sdNavigationCursor, 1 , &sdNavigationCallbackFilter, &sdNavigationCallbackLoad);
      }
      else
      {
        sdNavigationProcessFromEeprom(sdNavigationCursor, 1 , &sdNavigationCallbackLoadFromEeprom);
      }
      sdNavigationReset();
      delay(100);
      bdisplayed = false;
      return 2;//continue to new program

    }
  }//at least 1 file found

  if (checkKeybBreak() || (!(KEY[4] & BUTTON_0)) ||  (KEMPSTONJOYSTICK & BUTTON_LEFT)) //'0' key or left
  {
    waitforclearkeyb();
    sdNavigationReset();
    delay(100);
    bdisplayed = false;
    return -1;//back to old program
  }


  return 0;
}


void ICACHE_FLASH_ATTR sdNavigationSetup()
{
  //spi pin enabled in spiswitch
}








int ICACHE_FLASH_ATTR sdNavigationFileSave(char *filename) {

  SdFat sd;
  File file;

  sdNavigationLockSPI();


  if (!sd.begin(SD_CS, SPISettings(
                  SPI_SPEED_SD,
                  MSBFIRST,
                  SPI_MODE0))) {
    sdNavigationUnlockSPI();

    DEBUG_PRINTLN(F("LOCK KO"));
    return -1;
  }


  file = sd.open(filename, O_TRUNC | O_RDWR | O_CREAT   );
  if (!file.isOpen())
  {
    DEBUG_PRINTLN(F("OPEN KO"));
    sdNavigationUnlockSPI();
    return -1;
  }




  if ( z80FileSave(&file))
  {
    DEBUG_PRINTLN(F("WRITE KO AT EXIT"));
    sdNavigationUnlockSPI();
    return -1;
  }

  DEBUG_PRINTLN(F("WRITE OK"));
  sdNavigationUnlockSPI();
  return 0;
}



int ICACHE_FLASH_ATTR sdNavigationGetFileName(char *filename)
{
  int ioffset = 0;
  char c;


  UI_Cls(INK_BLACK);
  UI_PrintFStrBig(0, 0, F("Type file name"), COLOR_TEXT);
  UI_PrintFStrBig(0, 2, F("without ext"), COLOR_TEXT);

  waitforclearkeyb();


  for (;;)
  {
    RefreshScreen();
    if (checkKeybBreak())
    {
      waitforclearkeyb();
      return -1;
    }
    UI_PrintChBig(ioffset * 2, 6, '_', COLOR_TEXT + COLOR_BLINK);
    if (ioffset < 15) UI_PrintChBig(ioffset * 2 + 2, 6, ' ', COLOR_TEXT);
    c = getPressedCharacter();

    if (c == '\b' && ioffset) ioffset--; //delete

    if (ioffset < 15 && ( //new character
          (c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z') ||
          c == '_' || c == '-'))
    {
      UI_PrintChBig(ioffset * 2 , 6, c, COLOR_TEXT);
      filename[ioffset++] = c;
    }

    if (c == '\n')//enter
    {
      if (ioffset > 0)
      {
        filename[ioffset] = 0;
        return 0;
      }
      else
      {
        return -1;
      }
    }

  }


}
