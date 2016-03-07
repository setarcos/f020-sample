#include <C8051F020.h>
#include "pku.h"
#define uchar unsigned char
#define DELAY_LCD 100
void Lcd1602_init(void);
char isLcdBusy(void);
void Lcd1602_Write_Command(uchar Command_Code);
void Lcd1602_Write_Data(uchar Data);
void Lcd1602_Write_Char(uchar row, uchar column, uchar Data);
void Lcd1602_Write_String(uchar row, uchar column, uchar *dis_buffer);

uchar  __code table1[]={0x10,0x06,0x09,0x08,0x08,0x09,0x06,0x00};//字符℃
uchar  __code table2[]={0x04,0x1F,0x00,0x0A,0x0A,0x0A,0x1B,0x00};//字符六

void Lcd1602_init(void)
{
    Lcd1602_Write_Command(0x38);  // 8bit data, two lines, 5x7 font
    Lcd1602_Write_Command(0x08);  // display off
    Lcd1602_Write_Command(0x01);  // clear screen
    Lcd1602_Write_Command(0x06);  // cursor shift left
    Lcd1602_Write_Command(0x0c);  // display on
    Lcd1602_Write_Command(0x80);  // set data address
    Lcd1602_Write_Command(0x02);  // clear data pointer
}

/* Check where LCD is busy */
char isLcdBusy(void)
{
    P5 = 0xFF;
    P6 = 0x02; // RS=0, RW=1, EN=0
    Delay(DELAY_LCD);
    P6 = 0x03; // RS=0, RW=1, EN=1
    Delay(DELAY_LCD);
    return (P5 & 0x80);
}

void Lcd1602_Write_Command(uchar Command_Code)
{
    while(isLcdBusy());
    P5 = Command_Code;
    P6 = 0x00;         // RS=0,RW=0
    Delay(DELAY_LCD);
    P6 = 0x01;         // EN=1
    Delay(DELAY_LCD);
    P6 = 0x00;
}

void Lcd1602_Write_Data(uchar Data)
{
    P5 = Data;
    P6 = 0x04;                  // RS=1,RW=0
    Delay(DELAY_LCD);
    P6 = 0x05;                  // EN=1
    Delay(DELAY_LCD);
    P6 = 0x04;
}

void Lcd1602_Write_Char(uchar row, uchar column, uchar Data)
{
    while(isLcdBusy());

    if (row == 1)
        column |= 0xC0;
    else
        column |= 0x80;
    Lcd1602_Write_Command(column);
    Lcd1602_Write_Data(Data);
}

void Lcd1602_Write_String(uchar row, uchar column, uchar *dis_buffer)
{
    uchar i = 0;
    if (row >= 2) return; // is a 1602
    while (dis_buffer[i]!='\0') {
        Lcd1602_Write_Char(row, column + i, dis_buffer[i]);
        i++;
    }
}

void main(void)
{
    uchar i, j;
    WDTCN = 0xde;
    WDTCN = 0xad; // Disable watchdog
    SYSCLK_Init();
    P74OUT = 0x30;  // Port6 is push-pull
    Lcd1602_init();
 //   Lcd1602_Write_String(1, 8, "567");
    Lcd1602_Write_String(1, 1, "123");
    for (j = 0; j < 255; j++)
        Delay(10000);
    Lcd1602_Write_Command(0x40);
    for (i = 0; i < 8; i++) Lcd1602_Write_Data(table1[i]);
    for (i = 0; i < 8; i++) Lcd1602_Write_Data(table2[i]);
    for (i = 0; i < 8; i++) {
        Lcd1602_Write_Command(0x80);
        for (j = 0; j < 16; j++)
            Lcd1602_Write_Data(i * 32 + j);
        Lcd1602_Write_Command(0xC0);
        for (j = 0; j < 16; j++)
            Lcd1602_Write_Data(i * 32 + j + 16);
        for (j = 0; j < 255; j++)
            Delay(10000);
    }
    while(1);
}
