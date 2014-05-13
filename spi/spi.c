#include <c8051f020.h>
#include <stdio.h>
#include "pku.h"
#define SPICLK 2000000

void PORT_Init()
{
    XBR0 = 0x06;     // enable spi0 and uart0
    XBR2 = 0x40;     // enable xbr
    P0MDOUT |= 0x15; // TX, SCK, MOSI, as push-pull output
    P74OUT = 0x20; // P6H as push-pull, p6.7 is the spi_cs
}

void SPI0_Init()
{
    SPI0CFG = 0x07;  // 8bit words
    SPI0CN = 0x03;  // Master mode, spi enabled;
    SPI0CKR = SYSCLK/2/SPICLK;
}

unsigned char SPI_Write(unsigned char v)
{
    SPIF = 0;
    SPI0DAT = v;
    while (SPIF == 0);
    return SPI0DAT;
}

void Timer0_us (unsigned us)
{
    unsigned i;
    TCON &= ~0x30;
    TMOD &= ~0x0f;
    TMOD |= 0x01;
    CKCON |= 0x08;
    for (i = 0; i < us; ++i) {
        TR0 = 0;
        TH0 = (-SYSCLK/1000000) >> 8;
        TL0 = -SYSCLK/1000000;
        TR0 = 1;
        while (TF0 == 0);
        TF0 = 0;
    }
}


void putchar(char c)
{
    SBUF0 = c;
    while(!TI0);
    TI0 = 0;
}

void main()
{
    unsigned char v1;
    unsigned char v2;
    WDTCN=0XDE;
    WDTCN=0XAD;
    SYSCLK_Init();
    PORT_Init();
    P6 = 0x80;
    UART0_Init();
    SPI0_Init();
    Timer0_us(1000);
    P6 = 0x00;              // cs P6.7
    Timer0_us(1);           // cs setup
    SPI_Write(0x9F);        //read JEDEC ID
    v1 = SPI_Write(0x00);
    v2 = SPI_Write(0x00);
    printf("Manufacturer ID: %x\r\n", v1);
    printf("Memory Type ID: %x\r\n", v2);
    v2 = SPI_Write(0x00);
    printf("Capacity ID: %x\r\n", v2);
    Timer0_us(1);
    P6 = 0x80;
    while(1);
}

