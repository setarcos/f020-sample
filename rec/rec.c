#include <C8051F020.h>
#define SYSCLK 22118400
#define RATE    8192
#define COUNT   0x7000
#define UPSMP 4
#define THRESH 0x2000
#include "pku.h"

__xdata __at (0x8000) unsigned char seg;
__xdata __at (0x8001) unsigned char cs;
__xdata __at (0x0000) unsigned char mm[0x8000];

__code const unsigned char segs[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E};
__code const unsigned char css[] = {0x7, 0xB, 0xD, 0xE};

volatile unsigned samples;
volatile __bit adc1_done;

void ADC0_ISR (void) __interrupt (15)
{
    AD0INT = 0;
    if (adc1_done == 0) {
        unsigned k;
        k = ADC0;
        mm[samples] = k & 0xff;
        mm[samples + 1] = (k >> 8);
        samples += 2;
        if (samples >= COUNT) {
            adc1_done = 1;
            samples = 0;
            AD0EN = 0;
            DAC1CN |= 0x80;
        }
    }
}

void Timer4_ISR (void) __interrupt (16)
{
    T4CON &= ~0x80;
    if (adc1_done == 1) {
        unsigned k;
        k = mm[samples + 1] << 8;
        k += mm[samples];
        samples += 2;
        DAC1 = k;
        if (samples >= COUNT) {
            adc1_done = 0;
            samples = 0;
    //        DAC1CN &= ~0x80;
            AD0EN = 1;
        }
    }
}

void main(void)
{
    WDTCN = 0xde;
    WDTCN = 0xad; // Disable watchdog
    EMIF_Low();
    XBR0 = 0x04;     // uart0 enabled in xbr
    P0MDOUT |= 0x01; // set tx0 as push-pull
    Timer3_Init(SYSCLK/RATE);
    Timer4_Init(SYSCLK/RATE);
    ADC0_Init(1);
    DAC1CN = 0x17; // update on timer4 overflow, left justify
    adc1_done = 0;
    samples = 0;
    EA = 1;
    EIE2 |= 0x06; // enabled timer4 and adc0.1 interrupt


    while (1) {
        unsigned char i;
        unsigned j;
        if (adc1_done == 1)
            j = 0x0fff;
        else
            j = 0x0000;
        for (i = 0; i < 4; ++i) {
            seg = segs[j & 0xF];
            Delay(1);
            cs = css[i];
            j = j >> 4;;
            Delay(1000);
        }
    }
}
