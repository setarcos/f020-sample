#include <stdio.h>
#include <C8051F020.h>

#ifndef SYSCLK
#define SYSCLK 22118400
#endif
#ifndef BAUDRATE
#define BAUDRATE 115200
#endif
#define SPICLK 2000000

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

void busywait()
{
    unsigned char v;
    P6 = 0x0;
    Timer0_us(1);
    SPI_Write(0x05);
	v = 1;
    while(v & 0x01) {
        v = SPI_Write(0x00);
    }
    Timer0_us(1);
    P6 = 0x80;
}

void main()
{
    unsigned char v1, v2;
	long add;
	char c;
    WDTCN = 0xDE;
    WDTCN = 0xAD;
    SYSCLK_Init();
    PORT_Init();
    P6 = 0x80;
    UART0_Init();
	TI0 = 1;
    SPI0_Init();
    Timer0_us(1000);
    P6 = 0x00;              // cs P6.7
    Timer0_us(1);           // cs setup
    SPI_Write(0x9F);        //read JEDEC ID
    v1 = SPI_Write(0x00);
    v2 = SPI_Write(0x00);
    printf("Manufacturer ID: %bx\r\n", v1);
    printf("Memory Type ID: %bx\r\n", v2);
    v2 = SPI_Write(0x00);
    printf("Capacity ID: %bx\r\n", v2);
    Timer0_us(1);
    P6 = 0x80;

    while(1) {
	    do {
	    	c = getchar();
		} while ((c == ' ') || (c == '\r') || (c == '\n'));
        scanf("%lx", &add);
        switch (c) {
            case 'd':
                P6 = 0x00;
                Timer0_us(1);
                SPI_Write(0x03); // read data
                SPI_Write((add & 0x00FF0000) >> 16);
                SPI_Write((add & 0x0000FF00) >> 8);
                SPI_Write(add & 0x00FF);
                printf("\r\n");
                for (v1 = 0; v1 < 16; v1++) {
                    v2 = SPI_Write(0x00);
                    printf("%2bX ", v2);
					if (v1 == 7) printf("- ");
                }
                Timer0_us(1);
                P6 = 0x80;
				printf("\r\nD %lx OK\r\n", add);
                break;
            case 'w':
                scanf("%bd", &v1); // data
                P6 = 0x00;
                Timer0_us(1);
                SPI_Write(0x06); // write enable
				Timer0_us(1);
				P6 = 0x80;
				Timer0_us(1);
				P6 = 0x0;
				Timer0_us(1);
                SPI_Write(0x02);
                SPI_Write((add & 0x00FF0000) >> 16);
                SPI_Write((add & 0x0000FF00) >> 8);
                SPI_Write(add & 0x00FF);
                SPI_Write(v1);
				Timer0_us(1);
				P6 = 0x80;
				Timer0_us(1);
                busywait();
                printf("\r\nW %lx %bd OK\r\n", add, v1);
                break;
            case 'c':
                P6 = 0x00;
                Timer0_us(1);
                SPI_Write(0x06); // write enable
				Timer0_us(1);
				P6 = 0x80;
				Timer0_us(1);
				P6 = 0x0;
				Timer0_us(1);
                SPI_Write(0x20);
                SPI_Write((add & 0x00FF0000) >> 16);
                SPI_Write((add & 0x0000FF00) >> 8);
                SPI_Write(add & 0x00FF);
				Timer0_us(1);
				P6 = 0x80;
				Timer0_us(1);
                busywait();
                printf("\r\nC %lx OK\r\n", add);
                break;
            default:
                printf("Wrong command:%c\r\n", c);
        }
    }
}

