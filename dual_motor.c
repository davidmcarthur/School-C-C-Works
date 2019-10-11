
#include <hidef.h>           /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

// Function Prototypes
// Init Functions
void init_PWM(void);
void initPorts(void);

// LCD
void initLCD(void);
void LCDputs(char*);
void cmdwrt(unsigned char);		 
void datawrt(char);
void position(int,int);		 
// Delay
void delay1u(void);		
void delay100u(void);	
void delay2m(void);		
void delays(int);

#define Driver 0x01
#define Passenger 0x02       

#define Driver_On 0x08
#define Passenger_On 0x02

#define Driver_DIR 0x20
#define Passenger_DIR 0x10

const unsigned char cInit_commands[20] = {0x30,0x30,0x30,0x20,0x20,0x90,0x10,0x50,0x70,0x80,0x50,0xE0,0x60,0xA0,0x00,0xE0,0x00,0x10,0x00,0x60};

const unsigned char cE  = 0x04;
const unsigned char cRS = 0x01;


void main(void) 
{
  unsigned char sensor, old_sensor;
  
  initPorts( );	                        // port initializations for LCD
  initLCD( );	                          // LCD initialization
  DDRB |= 0x30;                         // Let PB4, PB5 becomes OUTPUT
  DDRT = 0xF0;                          // PORT T7-T4 output (LEDs)
  PTT = 0xF0;                           // initialize with LEDs off
  PORTB |= Driver_DIR;                  // Set driver motor direction
  PORTB &= ~Passenger_DIR;              // Set passenger motor direction
  
  DDRA = 0x00;                           // PORTA as inputs
  init_PWM();
 
  PWME = (Driver_On | Passenger_On);    // Turn both motors on. Channel 1 and 3 (PWM) initially
 
  for(;;) 
  {
    sensor = PORTA & 0x03;              // bit 0 is for driver, bit 1 for passenger
    
    if (old_sensor != sensor) 
    {
        
      switch(sensor)  
      {  
        case 0x00:        			            // Both sensors are off.
          PWME = (Driver_On | Passenger_On);// Turn both motors on. Channel 1 and 3
          cmdwrt(0x00); cmdwrt(0x10);
          position(1,0);
          LCDputs("Straigt Forward!");
    	      break;
    	     
        case Driver:     			            // Driver sensor is on.  
    	    PWME |= Driver_On;  	          // Turn driver motor on, passenger motor off.
    	    PWME &= ~Passenger_On;
    	    cmdwrt(0x00); cmdwrt(0x10);
          position(1,0);
          LCDputs("Turn Right !");
    	      break;
    	     
    	  case Passenger:                   // Passenger sensor is on.
          PWME |= Passenger_On;  	        // Turn Passenger motor on, Driver motor off.
          PWME &= ~Driver_On;
          cmdwrt(0x00); cmdwrt(0x10);
          position(1,0);
          LCDputs("Turn Left !");
    	      break;      
           
        case (Driver + Passenger):              //Both sensors are on.
          PWME &= ~(Driver_On | Passenger_On);  // Turn both motors off.      
          cmdwrt(0x00); cmdwrt(0x10);
          position(1,0);
          LCDputs("Stop !");
    	      break;

      }
      
      
    old_sensor = sensor;                // update the old_sensor to be used for next detection  
      
    }
    
  }

}

void init_PWM()
{
  PWMCAE = 0x00;                // left aligned
  PWMCLK = 0x00;                // Ch. 0 - Ch. 1 source is clock A, ch.2 & ch 3 use clock B 
  PWMCLKAB = 0x00;              // Use clock A and B respectively
  PWMPOL = 0x0A;                // initial HIGH output on ch. 1 and 3 (Use odd # registers)
  PWMPRCLK = 0x33;              // Clk A pre-scale = 8 (PWM clock = 6.25MHz/8 = 781.25 KHz =>period = 1.28uS), so is clock B
  PWMCTL = 0x30;                // CON01 = '1' and Con23 = '1': 16-bit PWM counter, period and duty regs.
  PWMPER01 = 600;               // obtain a period width about 770uS to create 1300Hz PWM
  PWMPER23 = 600;               // obtain a period width about 770uS to create 1300Hz PWM
  PWMDTY01 = 150;               // 25% duty cycle
  PWMDTY23 = 150;               // 25% duty cycle
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




