#include <C8051F020.h>
#include "pku.h"
#include "font.h"

__xdata __at (0x8003) unsigned char reset;
__xdata __at (0x8002) unsigned char cmd;
__xdata __at (0x9002) unsigned char data;

/*
void lcd_reset(void)
{
    P2 = 0x03;
    P1 = 0x80;
    Delay(10);
    P2 = 0x10;
}

#define NOP \
    __asm \
    nop \
    nop \
    nop \
    nop \
    __endasm
void wr_comm(unsigned char c)
{
    P2 = 0x02;
    P1 = 0x80;
    P3 = c;
    P0_7 = 0;
    P0_7 = 1;
    P2 = 0x10;
}

void wr_dat(unsigned char d)
{
    P2 = 0x02;
    P1 = 0x90;
    P3 = d;
    NOP;
    P0_7 = 0;
    NOP;
    P0_7 = 1;
    NOP;
    P2 = 0x10;
} */

void lcd_init9481(void)
{
    reset = 1;
    Delay(200);
    cmd = 0x11; // Exit from sleeping
    Delay(3000);
    cmd = 0xD0; // Power Setting
    data = 0x07;
    data = 0x41;
    data = 0x1D;
    data = 0x0D;

    cmd = 0xD1; // VCOM Control
    data = 0x00;
    data = 0x2B;
    data = 0x1F;

    cmd = 0x0C; // get pixel format (why get?)
    data = 0x55;
    cmd = 0x3A; // set pixel format
    data = 0x55; // 16bit/pixel
    cmd = 0xB4; // Display mode;
    data = 0;

    cmd= 0xC0; // Panel Driving Setting
    data = 0;
    data = 0x3B;
    data = 0x0;
    data = 0x2;
    data = 0x11;
    data = 0;

    cmd = 0xC5; // Frame rate and Inversion Control
    data = 0x03;

    cmd = 0xC8;  // Gamma Setting
    data = 0;
    data = 14;
    data = 0x33;
    data = 0x10;
    data = 0x00;
    data = 0x16;
    data = 0x44;
    data = 0x36;
    data = 0x77;
    data = 0x00;
    data = 0x0F;
    data = 0x00;

    cmd = 0xF3;
    data = 0x40;
    data = 0x0A;

    cmd = 0x36; // Address Mode
    data = 0x0A;

    cmd = 0xF0;
    data = 0x08;

    cmd = 0xF6;
    data = 0x84;
    cmd = 0xF7;
    data = 0x80;
    cmd = 0x36;  // Address Mode;
    data = 0x0A;

    Delay(3000);
    cmd = 0x29;  // Set display on
}

void lcd_init9486(void) /* ZHA */
{
    reset = 1;
    Delay(200);
    cmd = 0xF2;
    data = 0x18;
    data = 0xA3;
    data = 0x12;
    data = 0x02;
    data = 0xB2;
    data = 0x12;
    data = 0xFF;
    data = 0x10;
    data = 0x00;
    cmd = 0xF8;
    data = 0x21;
    data = 0x04;
    cmd = 0xF9;
    data = 0x00;
    data = 0x08;
    cmd = 0x36;
    data = 0x08;
    cmd = 0x3A;
    data = 0x05;
    cmd = 0xB4;
    data = 0x01;//0x00
    cmd = 0xB6;
    data = 0x02;
    data = 0x22;
    cmd = 0xC1;
    data = 0x41;
    cmd = 0xC5;
    data = 0x00;
    data = 0x07;//0x18
    cmd = 0xE0;
    data = 0x0F;
    data = 0x1F;
    data = 0x1C;
    data = 0x0C;
    data = 0x0F;
    data = 0x08;
    data = 0x48;
    data = 0x98;
    data = 0x37;
    data = 0x0A;
    data = 0x13;
    data = 0x04;
    data = 0x11;
    data = 0x0D;
    data = 0x00;
    cmd = 0xE1;
    data = 0x0F;
    data = 0x32;
    data = 0x2E;
    data = 0x0B;
    data = 0x0D;
    data = 0x05;
    data = 0x47;
    data = 0x75;
    data = 0x37;
    data = 0x06;
    data = 0x10;
    data = 0x03;
    data = 0x24;
    data = 0x20;
    data = 0x00;
    cmd = 0x11;
    Delay(200);
    cmd = 0x29;
}

/*
void lcd_init(void)
{
  wr_comm(0x11);
  Delay(4000);
  wr_comm(0xD0);
  wr_dat(0x07);
  wr_dat(0x41);    
  wr_dat(0x1D);
  wr_dat(0x0D);
    
  wr_comm(0xD1);
  wr_dat(0x00);
  wr_dat(0x2B);
  wr_dat(0x1F);

  wr_comm(0x0C);
  wr_dat(0x55);
  wr_comm(0x3A);
  wr_dat(0x55);            //  wr_dat(0x3A);
  wr_comm(0xB4);
  wr_dat(0x00);

  wr_comm(0xC0);
  wr_dat(0x00);
  wr_dat(0x3B);
  wr_dat(0x00);
  wr_dat(0x02);
  wr_dat(0x11);
  wr_dat(0x00);
  
  wr_comm(0xC5);
  wr_dat(0x03);
   
  wr_comm(0xC8);
  wr_dat(0x00);
  wr_dat(0x14);
  wr_dat(0x33);
  wr_dat(0x10);
  wr_dat(0x00);
  wr_dat(0x16);
  wr_dat(0x44);
  wr_dat(0x36);
  wr_dat(0x77);
  wr_dat(0x00);
  wr_dat(0x0F);
  wr_dat(0x00);
  
  wr_comm(0xF3);
  wr_dat(0x40);
  wr_dat(0x0A);
  
  wr_comm(0x36);
  wr_dat(0x0A);
  
  wr_comm(0xF0);
  wr_dat(0x08);
  wr_comm(0xF6);
  wr_dat(0x84);
  wr_comm(0xF7);
  wr_dat(0x80);
  wr_comm(0x36);
  wr_dat(0x0a);//0a
  
  Delay(12000);
  wr_comm(0x29);

} */

void dis_color(unsigned int c)
{
    int i, j;
     cmd = 0x2C; // start write
    for (i = 0; i < 480; ++i)
        for (j = 0; j < 320; ++j) {
            data = c >> 8;
            data = c & 0xFF;
        }
}

void show_char(unsigned int x, unsigned int y, unsigned int color, unsigned char f)
{
    unsigned char i, j, c;
    cmd = 0x2A; // set column address
    data = (x >> 8) & 1;
    data = (x & 0xFF);
    data = ((x + 7) >> 8) & 1;
    data = (x + 7) & 0xFF;
    cmd = 0x2B; // set page address
    data = (y >> 8) & 1;
    data = (y & 0xFF);
    data = ((y + 11) >> 8) & 1;
    data = (y + 11) & 0xFF;
    cmd = 0x2C;
    for (i = 0; i < 12; ++i) {
        c = font_8x12[f][i];
        for (j = 0; j < 8; ++j) {
            if (c & 1) {
                data = color >> 8;
                data = color & 0xFF;
            } else {
                data = 0xff;
                data = 0;
            }
            c = c >> 1;
        }
    }
}

void main(void)
{
    WDTCN = 0xde;
    WDTCN = 0xad; // Disable watchdog
    /*SYSCLK_Init();
    XBR2 = 0x40;
    P0MDOUT = 0xC0;
    P1MDOUT = 0xFF;
    P2MDOUT = 0xFF;
    P3MDOUT = 0xFF;
    P0 = 0xFF;
    P1 = 0xFF;
    P2 = 0xFF;
    P3 = 0xFF;
    lcd_reset(); */
    EMIF_Low();
    EMI0TC = 0x41;
    lcd_init9486();
    dis_color(0xFF00);
    show_char(160, 240, 0x00FF, 'F');
    show_char(168, 240, 0x00FF, '/');
    show_char(176, 240, 0x00FF, '0');
    while (1) {
    }
}
