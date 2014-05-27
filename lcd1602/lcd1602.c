#include <C8051F020.h>
#include "pku.h"
#define uchar unsigned char
void Lcd1602_init(void);
char isLcdBusy(void);
void Lcd1602_Write_Command(uchar Command_Code);
void Lcd1602_Write_Data(uchar row,uchar column,uchar Data);
void Lcd1602_Write_String(uchar row,uchar column,uchar *dis_buffer);
void Lcd1602_Clear_Char(uchar row,uchar column);
void Lcd1602_Clear_Line(uchar line);
void Lcd1602_Clear_All(void);

void Lcd1602_init(void)
{  
    Lcd1602_Write_Command(0x38);  // 8bit data, two lines, 5x7 font
    Lcd1602_Write_Command(0x08);  // display off
    Lcd1602_Write_Command(0x01);  // clear screen
    Lcd1602_Write_Command(0x06);  // cursor shift left
    Lcd1602_Write_Command(0x0c);  // display on
    Lcd1602_Write_Command(0x80);  // set data address
    Lcd1602_Write_Command(0x02);  // ckear data pointer
}    

/* Check where LCD is busy */
char isLcdBusy(void)
{
    P5 = 0xFF;
    P6 = 0x02; // RS=0, RW=1, EN=0
    Delay(100);
    P6 = 0x03; // RS=0, RW=1, EN=1
    Delay(100);
    return (P5 & 0x80);
}

void Lcd1602_Write_Command(uchar Command_Code)
{
    while(isLcdBusy());
    P6 = 0x00;         // RS=0,RW=0
    P5 = Command_Code;
    Delay(100);
    P6 = 0x01;         // EN=1	
    Delay(100);
    P6 = 0x00;         // EN=0           
}

void Lcd1602_Write_Data(uchar row,uchar column,uchar Data)
{
    while(isLcdBusy())

    if (row==2) 
        column |= 0x40;
    column |= 0x80;
    Lcd1602_Write_Command(column);
    P6 = 0x04;                  // RS=1,RW=0   
    P5 = Data;    				 
    Delay(100);				 
    P6 = 0x05;                  // EN=1	  
    Delay(100);				 
    P6 = 0x04;                  // EN=0	    
}

void Lcd1602_Write_String(uchar row,uchar column,uchar *dis_buffer)
{
    uchar i=0;
    if (row >= 2) return; // is a 1602
    while(dis_buffer[i]!='\0')
    {
        Lcd1602_Write_Data(row,column,dis_buffer[i]);
        i++;
        column++;
        if(column == 40)
        {               
            column=1;
            row++;
            if(row >= 2)
                return;  // is a 1602
            Lcd1602_Write_Command(0x80 + (row - 1) * 0x40);
        }
    } 
}

void main(void)
{
    WDTCN = 0xde;
    WDTCN = 0xad; // Disable watchdog
    SYSCLK_Init();
    P74OUT = 0x30;  // Port6 is push-pull
    Lcd1602_init();
    Lcd1602_Write_String(1, 4, "12345678");
    while(1);
}
