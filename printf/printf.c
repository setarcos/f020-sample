#include <c8051f020.h>
#include <stdio.h>
#define SYSCLK 22118400
#define BAUDRATE 9600
#include "pku.h"

void PORT_Init()
{
    XBR0 = 0x04;     // uart0 enabled in xbr
    XBR2 = 0x40;     // enable xbr
    P0MDOUT |= 0x01; // set tx0 as push-pull
}

void UART0_Init (void)
{
    SCON0 = 0x50;
    TMOD = 0x20;       // timer1 as baudrate generator
    TH1 = -(SYSCLK/BAUDRATE/16/12);
    TR1 = 1;            // start timer1
    //CKCON |= 0x10;      // timer1 use sysclk
    PCON |= 0x80;       // SMOD0 = 1
}

void putchar(char c)
{
    SBUF0 = c;
    while(!TI0);
    TI0 = 0;
}

void main()
{
    WDTCN=0XDE;
    WDTCN=0XAD;
    SYSCLK_Init();
    PORT_Init();
    UART0_Init();
    printf("Hello world~\r\n");
    while(1);
}

