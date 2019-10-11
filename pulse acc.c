// Peter Han, DeVry University 3-10-2014 
// Use pulse accumulator to measure frequency
#include <hidef.h>            /* common defines and macros */
#include "derivative.h"       /* derivative-specific definitions */
#include <stdio.h>


const unsigned char cInit_commands[20] = {0x30,0x30,0x30,0x20,0x20,0x90,0x10,0x50,0x70,0x80,0x50,0xE0,0x60,0xA0,0x00,0xE0,0x00,0x10,0x00,0x60};
const unsigned char cE  = 0x04;
const unsigned char cRS = 0x01;

unsigned char flags;

#define One_Sec_Past 0x01


// Function prototypes
void CLOCK_SETUP(void);
void position(int, int);
void LCDputs(char *);
void initLCD(void);
void cmdwrt(unsigned char);
void datawrt(char);
void initPorts(void);
void delay1u(void);
void delay100u(void);
void delay2m(void);
void delays(int);


void main(void) 
{
  unsigned int uiAcc_Cnt;
  char dspl[7];

  CLOCK_SETUP();      // Setup to use the external crystal and BUS CLOCK 4MHz

  initPorts();
	TSCR1 = 0x90; 	    // enable timer with fast flag clear automatically
	TSCR2 = 0x07;       // no overflow interrupt, prescaler = 128
	TIOS |= 0x01;	      // select channel 0 for Output Compare
  
  DDRT &= ~0x80;      // reset the PT7 to input
  PACTL = 0x50;       // D6-PAEN=1(PA enabled), D5-PAMOD=0(event counter),D4-PEDGE=1(rising edge,), all others Low

  initLCD();          // Get LCD ready
  position(1,0);
  LCDputs("Frequency is:");

  PACNT = 0;          // Clear the Pulse Accumulator Counter
  TFLG1_C0F = 1;      // reset ch0 Int flag
  TIE = 0x01;         // OC ch0 interrupt
  TC0 = TCNT + 31250; // record counts for 1 second
                      // 4MHz / 128 = 31250 Hz , Simply use TC0 = TCNT + 31250 to obtain 1 second
  EnableInterrupts;
   
  while(1) 
  {
    
    if (flags & One_Sec_Past) 
    {
      // Update LCD display
      uiAcc_Cnt = PACNT;
      (void)sprintf(dspl,"%u",(uiAcc_Cnt)); // convert decimal number to ASCII string
      
      PACNT = 0;      // reset Pulse Accumulator to zero
      position(2,0);
      LCDputs("                ");        // Clear the second row in LCD
      position(2,0);
      LCDputs(dspl);
      LCDputs("Hz");

      flags &= ~One_Sec_Past;
    }
    
  // Do other tasks

  }

}

void position(int iRow_value, int iCol_value) 
{
     int iPos_h, iPos_l, iValue;
     
     if(iRow_value == 1) 
        iValue = (0x80+0x00);
     
     if(iRow_value == 2) 
        iValue = (0x80+0x10);
     
     if(iRow_value == 3) 
        iValue = (0x80+0x20);
     
     iPos_h = ((iValue + iCol_value) & 0xF0);
     iPos_l = ((iValue + iCol_value) & 0x0F) << 4;
     
     cmdwrt(iPos_h);
     cmdwrt(iPos_l);
}
        
//Sends a string of characters to the LCD;...  
void LCDputs(char *sptr)
{  
     while(*sptr)
     {                                //...the string must end in a 0x00 (null character)
        datawrt(*sptr);               // sptr is a pointer to the characters in the string
        ++sptr;
     }
}  
                                          
// sends initialization commands one-by-one
void initLCD( )
{
  unsigned char i;
  
  for (i=0;i<20;i++)
  {  
    cmdwrt(cInit_commands[i]);
  }  
}

// sends a control word to LCD bus
void cmdwrt(unsigned char cCtrlword)
{  
  PORTC = cCtrlword;   // output command onto LCD data pins
  PORTC = cCtrlword + cE;   // generate enable pulse to latch it (xxxx x100)
  
  delay1u( );    // hold it for 1us
  
  PORTC = cCtrlword;    // end enable pulse  (xxxx x000)
  
  delay2m();    // allow 2ms to latch command inside LCD
  
  PORTC = 0x00;
  
  delay2m();    // allow 2ms to latch command inside LCD
}

// sends the character passed in by caller to LCD 
void datawrt(char cAscii)
{                   
  char cAscii_high, cAscii_low;
  
  cAscii_high = (cAscii & 0xF0);
  cAscii_low  = (cAscii & 0x0F) << 4; // Shift left by 4 bits 
  PORTC = cAscii_high;                // output ASCII character upper nibble onto LCD data pins
  PORTC = cAscii_high + cRS + cE;     // generate enable pulse to latch it  (0xxx x101)
  
  delay1u( );                         // hold it for 1us
  
  PORTC = cAscii_high + cRS;          // end enable pulse   (0xxx x001)
  
  delay1u( );                         // hold it for 1us
  
  PORTC = cAscii_low;                 // output ASCII character lower nibble onto LCD data pins
  PORTC = cAscii_low + cRS + cE;      // generate enable pulse to latch it  (0xxx x101)
  
  delay1u( );                         // hold it for 1us
  
  PORTC = cAscii_low + cRS;           // end enable pulse   (0xxx x001)
  
  delay100u( );                       // allow 100us to latch data inside LCD
}

void initPorts( )
{ 
  unsigned char cValue;
  
  DDRB   = 0x80;                        //LCD CSB active low
  DDRC   = 0xFF;                        // PC7-PC4 - 4-bit LCD data bus, PC2 - E, PC1 - R/W~, PC0 - RS: all outputs
  DDRP   = 0x01;                        // Enable backlight PP0 as output
  PTP = 0x01;                           // Turn on the backlight
  // PTP_PTP0 = 1;
  cValue = PORTB;
  PORTB  = (cValue & ~0x80);            // LCD CSB (PORT B7) enabled with a logic low
}  
  

void delay1u( )
{
  unsigned int i;
  
  for(i=0;i<=0x0f;i++)
  { /* adjust condition field for delay time */
    asm("nop");
  }
}

void delay100u( )
{
  unsigned int i,j;
  
  for(i=0;i<=0x02;i++)
  {  /* adjust condition field for delay time */
    for(j=0;j<=0xff;j++)
    {
       asm("nop");
    }
  }
}

void delay2m( )
{
   unsigned int i,j; 
   
   for (i=0;i<=0x20;i++)
   { /* adjust condition field for delay time */
     for (j=0;j<=0xff;j++)
     {
      asm("nop");
    }     
   }
}   

 
void delays(int k )
{
   unsigned int i,j; 
   
   for (i=0;i<=k;i++)
   { /* adjust condition field for delay time */
     for (j=0;j<=0xff;j++)
     {
      asm("nop");
    }     
   }
}     

/******* Interrupt Service Routine ********/
#pragma CODE_SEG NON_BANKED 
interrupt ( ( (0x10000 - 0xFFEE) / 2 ) - 1 )  void isrOC_ch0(void)
{
   TC0 = TCNT + 31250; // reset for next second
   flags |= One_Sec_Past;
}

#pragma CODE_SEG DEFAULT
// Use external 8 MHz crystal and bypass PLL and get bus clock in 4MHz 
void CLOCK_SETUP() 
{
 
  CPMUSYNR = 0x01;                /* SYNDIV = 1, VCO frequency 32 - 48 MHz */       
  CPMUREFDIV = 0x80;              /* REFDIV = 0, REFCLK frequency 6 - 12 MHz */       
  CPMUPOSTDIV = 0x07;             /* POSTDIV = 7 */
  CPMUCLKS_PLLSEL = 1;            /* PLLSEL = 1 */
  while (!CPMUCLKS_PLLSEL);       /* Verify CPMUCLKS configuration */
  CPMUOSC_OSCE = 1;               /* Enable Oscillator OSCE = 1 */
  while(!CPMUFLG_UPOSC);          /* wait for OSC to stabilise */
  CPMUCLKS_PLLSEL = 0;            /* PLLSEL = 0 */
  while (CPMUCLKS_PLLSEL);        /* Verify CPMUCLKS configuration */
  
}
 