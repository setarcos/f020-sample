#include <C8051F020.h>

#define SYSCLK 22118400
#include "pku.h"
#define SAMPLERATE 10000

__xdata __at (0x8000) unsigned char seg;
__xdata __at (0x8001) unsigned char cs;

__code const unsigned char segs[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E};
__code const unsigned char css[] = {0x7, 0xB, 0xD, 0xE};
volatile unsigned samples;


void ADC0_ISR (void) __interrupt (15)
{
    static unsigned int count;
    count++;
    AD0INT = 0;
    if (count >= SAMPLERATE / 2) {
        samples = ADC0;
        count = 0;
    }
}

void main(void)
{
    WDTCN = 0xde;
    WDTCN = 0xad; // Disable watchdog
    EMIF_Low();
    Timer3_Init(SYSCLK/SAMPLERATE);
    ADC0_Init(0);
    EA = 1;
    EIE2 |= 0x02; // enabled adc interrupt

    while (1) {
        unsigned char i;
        unsigned j;
        j = samples;
        for (i = 0; i < 4; ++i) {
            seg = segs[j & 0xF];
            Delay(1);
            cs = css[i];
            j = j >> 4;;
            Delay(1000);
        }
    }
}
