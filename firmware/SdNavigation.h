#ifndef SDNAVIGATION_H
#define SDNAVIGATION_H 

void sdNavigationSetup(void);
int sdNavigation(boolean);
void sdNavigationPrintNumber(int indx, int color);  
void sdNavigationPrintNumberBig(int indx, int color);  
int sdNavigationFileSave(char *filename);
int sdNavigationGetFileName(char *filename);

#endif
