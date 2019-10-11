#include <hidef.h>            /* common defines and macros */
#include "derivative.h"       /* derivative-specific definitions */
#include <stdio.h>
#include <termio.h>

void initLCD(void);
void initPorts(void);
void cmdwrt(unsigned char);
void datawrt(char);
void delay1u(void);
void delay100u(void);
void delay2m(void);
void delays(int);
void LCDputs(char *);
void position(int, int); 
void initSCI(void);
//void writeSCI(char);
void InitFifo(void);
int PutFifo (char);
int GetFifo (char *);
char REC_SCI(void);
void initSCI(void);



#define FifoSize 16

char *PutPt;     	   	      /* put next ptr */
char *GetPt; 
      		 			            /* get next ptr */
char Fifo[FifoSize];	      /* statically allocated fifo data */
 

//globals - 4-bit initialization sequence for LCD  -  data: PC7,6,5,4  E: PC2 , RS: PC0
const unsigned char cInit_commands[20] = {0x30,0x30,0x30,0x20,0x20,0x90,0x10,0x50,0x70,0x80,0x50,0xE0,
                                          0x60,0xA0,0x00,0xE0,0x00,0x10,0x00,0x60};

const unsigned char cE  = 0x04;         // PC2
const unsigned char cRS = 0x01;         // PC0

#define BAUD    9600L         // Baud rate to be set at 9600 bits per second
#define SYSCLK  6250000L      // Default system clock is 6.25 MHz

void main(void) 
{

  char read_temp, butterGet, foo[10];
  unsigned char i; 

  initPorts( );                        // port initializations
  initLCD( );                          // LCD inititialization
  initSCI();

  cmdwrt(0x00);
  cmdwrt(0xC0);                        // disable the cursor

  InitFifo();
   
  for (;;)
  {
    read_temp = REC_SCI();
	
	// for RTI change to flag?
	  if (read_temp == '1')              // Get if '1' is entered, other will be puts.
	  {
	    i = GetFifo(&butterGet);
      position(1, (GetPt - Fifo - 1));     // Move cursor to first row and retrieve the data, remove x
	    datawrt(' ');                        // Because the pointer is post increment, so minus 1

      cmdwrt(0x90);
	    cmdwrt(0x00);
	    datawrt(butterGet);
	  
	    if (i == 0)
	      LCDputs("  Queue Empty");
	    else
        LCDputs("             ");	     
	  }
	  else
	  {
	    if (-1 != PutFifo(read_temp))
	    {
	      cmdwrt(0x90);
		    cmdwrt(0x00);
	      LCDputs("Error,Queue Full");
		    break;
	    }
	    else
	    {
		    i = PutPt - Fifo;
        position(1, i-1);                    // Find the first row location and add an "x"
		    sprintf(foo,"%d",i-1);               // Because the pointer is post increment, so minus 1
		    puts(foo);                         // Send its ASCII code back
		    datawrt('x');
		    
		    cmdwrt(0x90);                       // clear second row of LCD
	      cmdwrt(0x00);
	      LCDputs("                ");
	    } 
	  
	  }
   }
   
   
  for (;;) {
  }

}


void InitFifo(void) 
{ 

 
  PutPt = GetPt = &Fifo[0];	// init pointers to fifo[0]'s address

}


int PutFifo (char data) 
{ 
	
	  char *Ppt;
	 
	  Ppt = PutPt;
	  *Ppt++ = data;
	  printf("%d\n\r",*(Ppt-1));
	  
	  if(Ppt == &Fifo[FifoSize])
		  Ppt = &Fifo[0];
	  
	  if(Ppt == GetPt){
		  LCDputs("Buffer Full\n");
		return 0;
	  }
	  else{
		  PutPt = Ppt;
		  puts("Rcv'd\n\r");
		  return(-1);
	  }
 }


int GetFifo (char *datapt) 
{ 

  if(PutPt == GetPt)
	  return 0;
  else{
	  *datapt = *GetPt++;
	  if(GetPt == &Fifo[FifoSize]){
		  GetPt = &Fifo[0];
	  }
	  return(-1);
  }
}


void initSCI()
{	

	SCI0BD = (unsigned int)(SYSCLK/BAUD/16);    // Set Baud Rate for UART
	SCI0CR1 = 0;				                        // UART configuration: 8 data bits, No parity bit, 1 stop bit
	SCI0CR2 = SCI0CR2_TE_MASK | SCI0CR2_RE_MASK;// SCI Tx and Rx enabled
	DDR0AD_DDR0AD7 = 0;			                    // PAD15 is connected to RS232 tranceiver enable	
	PPS0AD_PPS0AD7 = 0;                         // pull-up port1AD15
	PER0AD_PER0AD7 = 1;			                    // Make PAD15 input and enable pullup to activate RS232
}

char REC_SCI()
{
 // Your work ??		
	char c;
	
	while (!SCI0SR1_RDRF);
	;;
	c = SCI0DRL;
	return c;
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
  cValue = PORTB;
  PORTB  = (cValue & ~0x80);            // LCD CSB (PORT B7) enabled with a logic low
  PORTB  = (cValue & ~0x80);            // LCD CSB (PORT B7) enabled with a logic low
  DDRP   = 0x01;
  PTP   |= 0x01;
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
  
  
