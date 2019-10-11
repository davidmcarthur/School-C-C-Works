#include <hidef.h>            /* common defines and macros */
#include "derivative.h"       /* derivative-specific definitions */
#include <mc9s12g128.h>     

// Function Prototypes
void init_PWM(void);
void delays(int);

#define Driver 0x01
#define Passenger 0x02       

#define Driver_On 0x08
#define Passenger_On 0x02

#define Driver_DIR 0x20
#define Passenger_DIR 0x10

//structure definition, type definition
const struct state{	
    char *CmdPt[4] ;            // outputs
    //signed char *CmdPt;
    const struct state *Next [4];		// next...
}

typedef  state_type;
#define goStraight 		&fsm[0]
#define goLeft		 	&fsm[1]
#define goRight		 	&fsm[2]
#define Stop		  	&fsm[3]

// functions to control motor outputs per table
/*
State    >>>> INPUT		Left Sensor		Right Sensor	Output
On Track			      	Off				Off				Both Motors 70%
Left of Track			    ON				OFF				Left 70%, Right 30%
Right of Track		  	OFF				ON				Left 30%, Right 70%
Off Track			      	ON				ON				Both Off
*/

void bothOn(void){
	// both sensors are off.
	PWME = (Driver_On | Passenger_On);// Turn both motors on. Channel 1 and 3
}

void driveLeft(void){
	// Driver sensor is on.  
	PWME |= Driver_On;  	          // Turn driver motor on, passenger motor off.
	PWME &= ~Passenger_On;				// change to % of PWM once working...
}

void driveRight(void){
	// Passenger sensor is on.
	PWME |= Passenger_On;  	        // Turn Passenger motor on, Driver motor off.
	PWME &= ~Driver_On;				// change to % of PWM once working...
}
void bothOff(void){
	//Both sensors are on.
	PWME &= ~(Driver_On | Passenger_On);  // Turn both motors off.   
}
 
state_type fsm[5] = { // Traf Lite sequence - a structure containing 4 structures
	{{(char *)&bothOn, (char *)&driveLeft, (char *)&driveRight, (char *)&bothOff}, {goStraight, goLeft, goRight, Stop}},        // go Straight

	{{(char *)bothOn, (char *)driveLeft, (char *)driveRight, (char *)bothOff}, {goStraight, goLeft, goRight, Stop}},     	// go Left
	{{(char *)bothOn, (char *)driveLeft, (char *)driveRight, (char *)bothOff}, {goStraight, goLeft, goRight, Stop}},        // go Right
	{{(char *)bothOn, (char *)driveLeft, (char *)driveRight, (char *)bothOff}, {goStraight, goLeft, goRight, Stop}}			// Stop.
  };

// Main()
void main(void) 
{
	  
	state_type *Ptr;		// setup a pointer to a state-type data structure...
	unsigned char input;
  Ptr = goStraight;		// initialize pointer

	DDRB |= 0x30;                         // Let PB4, PB5 becomes OUTPUT
										  // PB5 is driver CW/CCW; PB4 is passenger CW/CCW.
	PORTB |= Driver_DIR;                  // Set driver motor direction
	PORTB &= ~Passenger_DIR;              // Set passenger motor direction
	PWME = (Driver_On | Passenger_On);    // Turn both motors on. Channel 1 and 3 (PWM) initially

	for (;;)
	{
		input = PORTA & 0x03;              // bit 0 is for driver, bit 1 for passenger
		((void(*)())Ptr->Next[input])(); // cast pointer to call function one through six;
		delays(10);			// delay 10ms.
		Ptr = Ptr->Next[input];

	}
	} // End Main()

// loops = 1/8MHz = 0.125us * 7905 = 1ms
void delays(int k )
{
   unsigned int i,j,x; 
   
   for(x=0;x<=k;x++)  {
     for (i=0;i<=0x1F;i++)
     { 
       for (j=0;j<=0xFF;j++)
       {
        asm("nop");
       }     
     }
   }
}

//  PWM Initialization.
void init_PWM()
{
  PWMCAE = 0x00;                // left aligned
  PWMCLK = 0x00;                // Ch. 0 - Ch. 1 source is clock A, ch.2 (char *) ch 3 use clock B 
  PWMCLKAB = 0x00;              // Use clock A and B respectively
  PWMPOL = 0x0A;                // initial HIGH output on ch. 1 and 3 (Use odd # registers)
  PWMPRCLK = 0x33;              // Clk A pre-scale = 8 (PWM clock = 6.25MHz/8 = 781.25 KHz =>period = 1.28uS), so is clock B
  PWMCTL = 0x30;                // CON01 = '1' and Con23 = '1': 16-bit PWM counter, period and duty regs.
  PWMPER01 = 600;               // obtain a period width about 770uS to create 1300Hz PWM
  PWMPER23 = 600;               // obtain a period width about 770uS to create 1300Hz PWM
  PWMDTY01 = 150;               // 25% duty cycle
  PWMDTY23 = 150;               // 25% duty cycle
}
