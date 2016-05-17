#include <C8051F020.h>
#include "pku.h"
#define uchar unsigned char
#define DELAY_LCD 100
#define NOKEY 255
void Lcd1602_init(void);
char isLcdBusy(void);
void Lcd1602_Write_Command(uchar Command_Code);
void Lcd1602_Write_Data(uchar Data);
void Lcd1602_Write_Char(uchar row, uchar column, uchar Data);
void Lcd1602_Write_String(uchar row, uchar column, uchar *dis_buffer);

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
        if (column + i > 16) break;
    }
}

uchar getkey()
{
    uchar i;
    uchar key;
    __code const uchar dec[] = {0, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0};
    __code const uchar trans[] = {0xC, 9, 5, 1, 0xD, 0, 6, 2, 0xE, 0xA, 7, 3, 0xF, 0xB, 8, 4};
    P4 = 0x0F;            // 低四位高电平输出
    Delay(100);           // 延时等待电平稳定
    i = ~P4 & 0x0F;       // 获取低四位
    if (i == 0) return NOKEY; // 没有按键
    key = dec[i] * 4;     // 转换为列值然后x4
    Delay(1000);          // 延时去抖
    P4 = 0xF0;            // 高四位高电平输出
    Delay(100);           // 延时
    i = ~P4;
    i >>= 4;              // 获取输入值
    if (i == 0) return NOKEY; // 没有按键
    key = key + dec[i];   // 计算出键码
    key = trans[key];     // 转换为真实键值
    return key;
}

void calculate(uchar buf[])
{
    uchar p; // 字符索引
    uchar c;
    long a, b, r;
    p = 0;
    while (buf[p] == ' ') p++; // 去掉空格
    a = 0;
    b = 0;
    r = 0;
    if (buf[p] == '-') {
        p++;
        r = -1;
    }
    while ((buf[p] >= '0') && (buf[p] <= '9')) {
        a = a * 10 + buf[p] - '0';  // 计算第一个数
        p++;
    }
    if (r == -1) a = 0 - a;
    c = buf[p]; // 运算符
    p++;
    b = 0;
    while ((buf[p] >= '0') && (buf[p] <= '9')) {
        b = b * 10 + buf[p] - '0';  // 计算第二个数
        p++;
        if (p > 15) break;
    }
    switch (c) {
        case '+': r = a + b;
                  break;
        case '-': r = a - b;
                  break;
        case '*': r = a * b;
                  break;
        case '/': if (b == 0) r = 0;
                  else r = a / b;
                  break;
        default:
                  r = 0;
    }
    for (c = 0; c < 15; c++) buf[c] = ' ';
    if (r < 0) {
        buf[0] = '-'; // 记录符号
        r = 0 - r;
    }
    c = 15;
    while (r > 0) {
        buf[c] = r % 10 + '0';
        r = r / 10;
        c--;
    }
    if (buf[0] == '-') { // 移动符号
        buf[c] = '-';
        buf[0] = ' ';
    }
}

void main(void)
{
    uchar key;
    uchar oldkey;   // 旧按键
    uchar cal;      // 操作符
    uchar buf[16];  // 显示缓冲
    uchar i;
    __bit clear;
    __code const uchar token[4] = {'+', '-', '*', '/'};
    WDTCN = 0xde;
    WDTCN = 0xad; // Disable watchdog
    SYSCLK_Init();
    P74OUT = 0x30;  // Port6 is push-pull
    Lcd1602_init();
    Lcd1602_Write_String(0, 1, "Calculator");

    oldkey = NOKEY;
    cal = ' ';
    clear = 0;
    for (i = 0; i < 16; ++i) buf[i] = ' ';
    while (1) {
        key = getkey();
        if (key == NOKEY) oldkey = NOKEY;
        if (key == oldkey) continue;
        if (key < 10) {     // 数字
            if (buf[0] != ' ') continue; // 缓冲区满
            if ((buf[9] != ' ') && (cal == ' ')) continue; // 数字太大
            if (clear) // 刚按过等号
                for (i = 0; i < 16; ++i) buf[i] = ' ';
            for (i = 0; i < 15; ++i) buf[i] = buf[i + 1];
            buf[15] = key + '0';
        }
        if (key == 0xF) { // 退格
            if ((cal != ' ') && (buf[15] == cal)) cal = ' ';
            for (i = 15; i > 0; --i) buf[i] = buf[i - 1];
        } else if (key == 0xE) { // 等号
            if (cal != ' ') {
                calculate(buf);
                cal = ' ';
            }
        } else if (key > 9) { // 加减乘除
            if ((buf[15] == ' ') && (key == 0xB)) { // 负号，目前仅第一个操作数可以是负数
                buf[15] = '-';
            } else if ((buf[15] != cal) && (buf[15] != '-')) {   // 判断是否刚输入完运算符
                if (cal != ' ') calculate(buf);
                cal = token[key - 10];
                for (i = 0; i < 15; ++i) buf[i] = buf[i + 1];
                buf[15] = cal;
            }
        }
        Lcd1602_Write_String(1, 0, buf);
        oldkey = key;
        if (key == 0xE) clear = 1;
        else clear = 0;
    }
}
