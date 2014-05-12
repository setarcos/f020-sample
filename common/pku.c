#include "pku.h"
#include <C8051F020.h>
#ifndef SYSCLK
#define SYSCLK 22118400
#endif
#ifndef BAUDRATE
#define BAUDRATE 115200
#endif
/* simple for loop delay */
void Delay(int k)
{
    int i;
    for (i = 0; i < k; ++i);
}

void SYSCLK_Init()
{
    OSCXCN = 0x67;  // enabled external crystal
    while (!(OSCXCN & 0x80));  // wait for stable
    OSCICN = 0x88;  // use external crystal
}

/* init the emif using the lower port  */
void EMIF_Low(void)
{
    SYSCLK_Init();
    EMI0CF = 0x1F; // non-multiplexed mode, external only
    XBR2 = 0x42;    // Enable xbr
    P0MDOUT = 0xC0;
    P1MDOUT = 0xFF;
    P2MDOUT = 0xFF;
    P3MDOUT = 0xFF;
}

void Timer4_Init (int counts)
{
   T4CON = 0;                       // STOP timer; set to auto-reload mode
   CKCON |= 0x40;                   // T4M = ‘1’; Timer4 counts SYSCLKs
   RCAP4 = -counts;                 // set reload value
   TMR4 = RCAP4;
   EIE2 &= ~0x04;                    // disable Timer4 interrupts
   T4CON |= 0x04;                   // start Timer4
}

void Timer3_Init(int counts)
{
    TMR3CN = 0x02;
    TMR3RL = -counts;
    TMR3 = 0xffff;
    EIE2 &= ~0x01;
    TMR3CN |= 0x04;
}

void ADC0_Init (int mux)
{
    ADC0CN = 0x05;  // ADC0 disabled; normal tracking
                    // mode; ADC0 conversions are initiated
                    // on overflow of Timer3; ADC0 data is
                    // left-justified
    REF0CN = 0x03;  // enable on-chip VREF,
    AMX0SL = mux;     // Select AIN_k as ADC mux output
    ADC0CF = (SYSCLK/2500000) << 3; // ADC conversion clock = 2.5MHz
    ADC0CF &= ~0x07;     // PGA gain = 1
    EIE2 &= ~0x02;       // disable ADC0 interrupts
    AD0EN = 1;           // enable ADC0

}

void UART0_Init ()
{
    SCON0 = 0x50;
    TMOD = 0x20;       // timer1 as baudrate generator
#ifdef USE_CKCON
    CKCON |= 0x10;      // timer1 use sysclk
    TH1 = -(SYSCLK/BAUDRATE/16);
#else
    TH1 = -(SYSCLK/BAUDRATE/16/12);
#endif
    TR1 = 1;            // start timer1
    PCON |= 0x80;       // SMOD0 = 1
}
