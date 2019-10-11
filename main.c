// ECET340
// Professor Han

/* Connect the function generator synch output (TTL voltage levels) 
to Port T1 (pin 22 on the 40 pin connector) with respect 
to the Tower ground (pin 25 on the 40 pin connector).
Set the signal frequency to 100, 1K or 100K Hz. 
(Need to place signal type to square wave to adjust signal duty cycle.)
The program will display its frequency */


#include <hidef.h>             /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <stdio.h>

//globals
unsigned int iT1;	                         // time associated with rising edge of pulse
unsigned int iT2;	                         // time associated with falling edge of pulse
unsigned int iPulseWidth;	                 // time associated with signal pulse width
unsigned int iDisplay;
unsigned char character[5];

const unsigned char cInit_commands[20] = {0x30,0x30,0x30,0x20,0x20,0x90,0x10,0x50,0x70,0x80,0x50,0xE0,0x60,0xA0,0x00,0xE0,0x00,0x10,0x00,0x60};

const unsigned char cE  = 0x04;
const unsigned char cRS = 0x01;

unsigned char cValues;
int i=0;


//function prototypes
void initLCD(void);
void initPorts(void);
void LCDputs(char*);;
void cmdwrt(unsigned char);		 
void datawrt(char);
void position(int,int);		 
void delay1u(void);		
void delay100u(void);	
void delay2m(void);		
void delays(int);
void CLOCK_SETUP(void);

/******* main *******/
void main(void)
{
	char cnt[9];        // reserve a 5 digits number to display
	float freq, freq_average=0.0;

  CLOCK_SETUP();      // Setup to use the external crystal and BUS CLOCK 4MHz

	
	DDRT = 0xF0;        // Direction of Port T upper nibble as outputs (LEDs)
	TSCR1 = 0x80; 	    // enable timer 
	TSCR2 = 0x06;       // no overflow interrupt, prescaler = 64
	TIE = 0x00;         // no interrupt
	TIOS = TIOS & ~0x02;	 // select channel 1 for input capture (logic low)

  initPorts( );	      // port initializations
  initLCD( );	        // LCD inititialization
  
  TCTL4 = 0x04;                 // select channel 1 for capture on rising edge

  
	for(;;)                         // an infinite loop
	{   
		TFLG1_C1F = 1;                // clear channel 1 flag
		
		while (!TFLG1_C1F)            // wait for channel 1 first rising edge
		   ;;
		   
		TFLG1_C1F = 1;                // clear channel 1 flag
		iT1 = TC1;                    // get the TCNT value for channel 1 rising edge
		
		while (!TFLG1_C1F) ;           // wait for channel 1 rising edge
		   ;;
		   
		TFLG1_C1F = 1;                // clear channel 1 flag
		iT2 = TC1;                    // get the TCNT value for channel 1 rising edge AGAIN
		
		if (iT2 > iT1)
		  iPulseWidth  = iT2 - iT1;   // calculate the signal pulse width

   
   freq = (62500.00 / iPulseWidth);    // add xx.00 to make if float
   
   if (i < 3)                     // Average by adding 4 times and divide 4
   {
     i++;
     freq_average += freq;        // Add the first 3 times
   }
   else 
   {
     freq_average += freq;        // Add the fourth time
     freq_average /= 4;           // Divided by 4
     i = 0; 
     (void)sprintf(cnt,"%8.2f",freq_average);   // convert number to ASCII
     cmdwrt(0x00); cmdwrt(0x10);		// clear LCD
     position(1,0);   
     LCDputs(cnt);                  // Print frequency to the LCD
     LCDputs(" Hz");
     freq_average = 0;              // Reset average to 0
     delays(100);
   }
  
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
 
 