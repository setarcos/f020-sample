#include <C8051F020.h>
#include "pku.h"

void PORT_Init()
{
    P74OUT = 0x80;    // push-pull for P7.4-p7.7
}

void main()
{
    WDTCN=0xDE;
    WDTCN=0xAD;
    SYSCLK_Init();
    PORT_Init();

    EA = 1;       // enable interrupt
    TMOD |= 0x02; // timer0 8bit with auto-reload
    TH0 = 0x00;
    TL0 = 0x00;
    TR0 = 1;
    ET0 = 1;
    P7 = 0x80;
    while (1);
}

void TIMER0_ISR (void) __interrupt (1)
{
    static int count;
    count++;
    if (count > 3600){
        count = 0;
        P7 = ~P7;
    }
}
