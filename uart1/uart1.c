#include <c8051f020.h>

#define SYSCLK 22118400
#include "pku.h"

void PORT_Init()
{
    XBR0 = 0x04;     // uart0 enabled in xbr
    XBR2 = 0x40;     // enable xbr
    P0MDOUT |= 0x01; // set tx0 as push-pull
}

void main()
{
    WDTCN=0XDE;
    WDTCN=0XAD;
    SYSCLK_Init();
    PORT_Init();
    UART0_Init();
    while(1)
    {
        SBUF0 = 'A';
        while(!TI0);
        TI0=0;
    }
}

