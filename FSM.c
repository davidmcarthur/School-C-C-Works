// works, want to make some tweeks...
#include <hidef.h>            /* common defines and macros */
#include "derivative.h"       /* derivative-specific definitions */
#include <mc9s12g128.h>     


//structure definition, type definition
struct state{	
	unsigned char outputLED;
	unsigned int timeDelay;
	struct state *Next;  // pointer to the structure for the next state
}
typedef state_type;
#define S0  &fsm[0]
#define S1  &fsm[1]
#define S2  &fsm[2]
#define S3  &fsm[3]
#define gN  0x0C
#define yN  0x14
#define gE  0x21
#define yE  0x22
#define d6  6
#define d2  2
 
state_type fsm[4] = { // Traf Lite sequence - a structure containing 4 structures
    {gN,d6,S1},// N Green, E Red %001100  
    {yN,d2,S2},   // N Yell, E Red %010100
    {gE,d6,S3}, // N Red, E Green %100001
    {yE,d2,S0}, // N Red, E Yell %100010

};

static int counter = 0;
void delay(int); // use for testing
//void init_RTI(void); // initialize clock for RTI
// RTI Variables

 unsigned int rti1000ms;
 unsigned char flagSecond;      // A "1 second" flag

void main(void) 
{

  state_type *ptr;		// setup a pointer to a state-type data structure...

  ptr = S0;		// ...and initialize it to point to state S0 in fsm
  DDRA = 0x3F;		// PA0 - PA5 as outputs.
  PORTA = 0x0C;		// initialize PORTA to S0.

  ECLKCTL_NECLK = 0;
  CPMURTI = 0x80 | 0x00;    // Mode decimal, divider 1000              
  CPMUFLG_RTIF = 1;                    // clear RTIF before enable INT
  CPMUINT_RTIE = 1;	                  // enable RTI
  EnableInterrupts;                    // global enable
  flagSecond = 1;

   for (;;)
   {
		PORTA = ptr->outputLED;
		delay(ptr->timeDelay);
		ptr = ptr->Next;
   }
}

// delay function, change to RTI.
void delay(int x){ // delay for 'del' msec

  while(x!=0)
  if(flagSecond ==1) {
    x--;
    flagSecond = 0;
  }
}

/******* Interrupt Service Routine ********/
#pragma CODE_SEG NON_BANKED 
interrupt VectorNumber_Vrti  void isrRTI(void)
{
   rti1000ms++;
   if(rti1000ms>=1000)
   {
      rti1000ms = 0;
      
      flagSecond = 1;
   }

   CPMUFLG_RTIF = 1;   // clear RTIF
}

#pragma CODE_SEG DEFAULT  
