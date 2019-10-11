
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <stdio.h>

const struct state  {
  char *CmdPt[2];            // Two pointer in the array to char
  const struct state *Next [2];
};

typedef const struct state stateType;
#define happy  &fsm [0]
#define hungry &fsm [1]
#define sleepy &fsm [2]

void one (void){
  PORTA = 0x01;
}

void two (void){
  PORTA = 0x02;
}

void three (void){
  PORTA = 0x03;
}

void four (void){
  PORTA = 0x04;
}

void five (void){
  PORTA = 0x05;
}

void six (void){
  PORTA = 0x06;
}
               


stateType fsm[3] = { 
  {{(char *)one, (char *)two}, {hungry, happy}},         // happy
  {{(char *)three,(char *)four}, {hungry, sleepy}},      // hungry
  {{(char *)five, (char *)six}, {happy, hungry}}         // sleepy
  };
  
void delays (unsigned char);  

void main(void) {

stateType *Pt;
unsigned char input;

  DDRA = 0xFF;      // All outputs on PORTA
  DDRB = 0x00;      // All inputs on PORTB
  Pt = happy;

  while(1) {
    input = PORTB;
    ((void(*)())Pt->CmdPt[input])();
    delays (10);
    Pt = Pt->Next[input];
  }
}
// loops = 1/8MHz = 0.125us * 7905 = 1ms
void delays(int k )
{

}

/*
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

  */