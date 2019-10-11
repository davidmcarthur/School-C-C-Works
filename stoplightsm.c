//4-state Traffic Light Control using Linked ListFSM 
// Wytec HC12 Dev Board (24MHz eclk) using Codewarrior 
// Outputs: Connection of LED signals connected to PORT B 
// PB7   PB6   PB5   PB4      PB3   PB2   PB1   PB0
// RED   YEL   GRN   NC        NC   RED   YEL   GRN
// <-- North-South Road --> | <-- East-West Road --> |
/* Output Port: Port B - demo board LEDs (set PJ1=0) or...
         off-board Red-Yel-Grn LEDs (set PJ1=1) */ 

#include <hidef.h>      
#include <mc9s12dg256.h>     
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"

//structure definition, type definition
struct state{	// a linked structure: there will be one of these for each state
	unsigned char LED_out; // LED outputs
	int time;	// time to spend in this state
  struct state *Next[2];  // contains pointers to each of the two possible
				        // …structures for the next state

}

typedef state_type; // makes this structure reusable in other structures
//state 0 is defined by the 0th structure in fsm, state 1 by the 1st, etc.
#define S0  &fsm[0]	 		
#define S1  &fsm[1]
#define S2  &fsm[2]
#define S3  &fsm[3] 
#define S4  &fsm[4]

state_type fsm[5] = { // Traf Lite sequence - a structure containing 4 structures
    {0x28,5000,{S1,S1}},  // state S0's LED outputs, time in this state, ptr to next
    {0x48,1000,{S2,S4}},   // state S1's LED outputs, etc..
    {0x82,5000,{S3,S3}},
    {0x84,1000,{S0,S4}}, 
    {0x88,8000,{S0,S0}},     
    };
     
void delay(int);
void SW5_INT(void);
void cross_seq(void);

 state_type *ptr;	// setup a pointer to a state-type data structure...
 int ptr_index = 0;  // offset into array *Next[2]; value is either 0 or 1
/************ main ************/
void main( ){

   // state_type *ptr;	// setup a pointer to a state-type data structure...
    //int ptr_index = 0;  // offset into array *Next[2]; value is either 0 or 1
    ptr = S0;		// ...and initialize it to point to state S0 in fsm
   
    DDRJ |= 0x02;	// PJ1 outputs
    PTJ &= ~0x02;   // PJ1 = 0  to enable on-board LEDs for use with Port B
    DDRB = 0xFF;  // Port B outputs
 	  SW5_INT();
 	  
    for(;;)/> {
	PORTB = ptr->LED_out;  // send out LED outputs for present state
	delay(ptr->time);  // wait in the present state before entering the next
	ptr = ptr->Next[ptr_index];  // point to the next state in the sequence
    }                  
}

void delay(int del){ // delay for 'del' msecs
  int i,j;
 for(i=0; i<del; i++)
    for(j=0; j<4000; j++);
}

void SW5_INT(){
    asm("sei");
    DDRH &= 0x01; //PH0 as key SW5
    PPSH = 0xF0;  //enable pull-up PH3 - PH0, define FE trigger for interrupt
    PERH |= 0x0F;
    PIFH |= 0x00;
    PIEH |= 0x01;  //enable PH0 for port interrupt
    asm("cli");
}

void cross_seq(void){
  if(ptr==S1)   
	   ptr_index = 1;    // point to R-R State
      else
	   ptr_index = 0;
    
  if(ptr==S3)   
	   ptr_index = 1;    // point to R-R State
      else
	   ptr_index = 0;

ptr = ptr->Next[ptr_index]; // advances pointer to the next state...
 				  // ... in the sequence
  }


    
#pragma CODE_SEG NON_BANKED
interrupt 25 void PortH_ISR (void)	  
//this section of code defines the "interrupt service routine"
	{
	delay(20);      //For switch debouncing
	cross_seq();
  	PIFH |=  0x0F;  //clear interrupt flag before returning out to main program
}
#pragma CODE_SEG DEFAULT

