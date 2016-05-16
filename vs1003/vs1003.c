#include <C8051F020.h>
#include <stdio.h>
#include "pku.h"
#include "music.h"
#define SPICLK 2000000

void PORT_Init()
{
    XBR0 = 0x02;     // enable spi0
    XBR2 = 0x40;     // enable xbr
    P0MDOUT |= 0x05; // SCK, MOSI, as push-pull output
    P74OUT = 0x80; // P7H as push-pull
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

/*
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
*/

void wr_cmd(unsigned char addr, unsigned char hdat, unsigned char ldat)
{
    P6 = 0x40;  // P6.6 is DREQ
    while ((P6 & 0x40) == 0);
    P7 &= ~0x20;  // P7.5 is XCS, P7.6 is DCS
    SPI_Write(0x02);
    SPI_Write(addr);
    SPI_Write(hdat);
    SPI_Write(ldat);
    P7 |= 0x20;
}

void Mp3Reset(void)
{
    P7 = 0x60;  // P7.4 is RESET
    Delay(100);
    P7 = 0x70;  // DCS = XCS = RESET = 1
    wr_cmd(0x00, 0x08, 0x04);
    Delay(10);
    P6 = 0x40;  // DREQ = 1;
    while ((P6 & 0x40) == 0);
    wr_cmd(0x03, 0x98, 0x00);
    Delay(10);
    wr_cmd(0x05, 0xbb, 0x81);
    Delay(10);
    wr_cmd(0x02, 0x00, 0x55);
    Delay(10);
    wr_cmd(0x0b, 0x04, 0x04);  // Volumn
    Delay(10);
    SPI_Write(0);
    SPI_Write(0);
    SPI_Write(0);
    SPI_Write(0);
}

void send_dat(unsigned char dat)
{
    P6 = 0x40;  // P6.6 is DREQ
    while ((P6 & 0x40) == 0);
    SPI_Write(dat);
}

void flush_buffer()
{
    unsigned int i;
    P7 &= ~0x40;
    for (i = 0; i < 2048; ++i)
        send_dat(0);
    P7 |= 0x40;
}

void main()
{
    unsigned int i;
    WDTCN=0xDE;
    WDTCN=0xAD;
    SYSCLK_Init();
    PORT_Init();
    SPI0_Init();

    Mp3Reset();
    P7 &= ~0x40;
    for (i = 0; i < 15000; ++i)
        send_dat(music[i]);
    flush_buffer();
    while(1);
}

