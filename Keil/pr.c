#include <stdio.h>
#include <C8051F020.h>

#ifndef SYSCLK
#define SYSCLK 22118400
#endif
#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

void SYSCLK_Init()
{
	int i;
    OSCXCN = 0x67;  // enabled external crystal
	for (i=0; i < 256; i++);
    while (!(OSCXCN & 0x80));  // wait for stable
    OSCICN = 0x88;  // use external crystal
}

void PORT_Init()
{
    XBR0 = 0x04;     // uart0 enabled in xbr
    XBR2 = 0x40;     // enable xbr
    P0MDOUT |= 0x01; // set tx0 as push-pull
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

void main()
{
    char a, b, c, e;
    WDTCN = 0xDE;
    WDTCN = 0xAD;
    SYSCLK_Init();
    PORT_Init();
    UART0_Init();
	TI0 = 1;
    printf("Hello world~\r\n");
    while(1) {
        scanf("%bd%c%bd", &a, &b, &c);
        switch (b) {
            case '+': e = a + c;
                      break;
            case '-': e = a - c;
                      break;
            case '*': e = a * c;
                      break;
            case '/': e = a / c;
                      break;
            default:
                      e = 0;
        }
        printf("%bd%c%bd=%bd", a, b, c, e);
    }
}

