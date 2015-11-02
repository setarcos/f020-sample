#include <C8051F020.h>              // SFR declarations
#define SYSCLK 22118400             // SYSCLK frequency in Hz
#define SAMPLERATED 8000         // update rate of DAC in Hz
#include "pku.h"

void Timer4_ISR (void) __interrupt (16)
{
   static unsigned char phase = 0;
   const unsigned data[] = {32768, 37831, 42398, 46023, 48350, 49152, 48350, 46023, 42398, 37831, 32768, 27705, 23138, 19513, 17186, 16384, 17186, 19513, 23138, 27705};
   DAC1 = data[phase % 20] / 2;
   phase += 1;
   if (phase == 20) phase = 0;
   T4CON &= ~0x80;
}

void main (void) {
   WDTCN = 0xde;                    // Disable watchdog timer
   WDTCN = 0xad;
   EMIF_Low();
   REF0CN = 0x03;                   // enable internal VREF generator
   DAC1CN = 0x94;                   // enable DAC0 in left-justified mode
                                    // using Timer4 as update scheduler
   Timer4_Init(SYSCLK/SAMPLERATED); // initialize T4 to generate DAC0
                                    // schedule
   EA = 1;                          // Enable global interrupts
   EIE2 |= 0x04;                    // enable Timer4 interrupts
   while (1);                       // spin forever
}
