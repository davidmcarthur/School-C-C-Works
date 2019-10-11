#include <hidef.h>
#include "derivative.h"
#include <stdio.h>
#include "..\common\lcd.h"
#include "..\common\delay.h"

unsigned int iCount;	  // count variable
unsigned int iValue;   // LED display variable
char str[10]={ 0 };



void main(void)
{
   DDRT = 0x70;   // Port T7 input (PA Input)
                  // PT6, PT5, PT4 (LEDs)

   PACTL = 0x50; 	// D6-PAEN=1(PA enabled)
                  // D5-PAMOD=0(event counter),
                  // D4-PEDGE=1(rising edge,)
                  // all others logic Low
                  
	lcdPortInit();  // port initializations
	lcdInit();	     // LCD initialization
	lcdCursor(CURSOR_OFF);
	lcdPosition(1,0);
	lcdPuts("iLab4a");
	lcdPosition(2,0);
	lcdPuts("David McArthur");
	lcdPosition(3,0);
	lcdPuts("Ticks: ");

  for(;;)
  {   
      iCount = PACNTL;      //obtain current time
      
      iValue = (~(iCount <<4) ) & 0x70;   
      // shift iCount left 4 digits and mask for D6-D4
      
      PTT = (char)iValue;    //output result to LEDs – active Low
	  (void)sprintf(str,"%d",iCount);
	  lcdPosition(3,7);
	  lcdPuts(str);
  }
}