#include <c8051f020.h>              // SFR declarations
#define SYSCLK 22118400             // SYSCLK frequency in Hz
#define SAMPLERATED 10000L         // update rate of DAC in Hz
#include "pku.h"

void Timer4_ISR (void) __interrupt (16)
{
   static unsigned int phase = 0;
   DAC0 = phase;
   phase += 0x08;
   T4CON &= ~0x80;      // clear interrupt flag
}

void main (void) {
   WDTCN = 0xde;                    // Disable watchdog timer
   WDTCN = 0xad;
   EMIF_Low();
   REF0CN = 0x03;                   // enable internal VREF generator
   DAC0CN = 0x97;                   // enable DAC0 in left-justified mode
                                    // using Timer4 as update scheduler
   Timer4_Init(SYSCLK/SAMPLERATED); // initialize T4 to generate DAC0
                                    // schedule
   EA = 1;                          // enable global interrupts
   EIE2 |= 0x04;                    // enable Timer4 interrupts
   while (1);                       // spin forever
}

