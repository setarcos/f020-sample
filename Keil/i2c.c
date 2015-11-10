/* The ISR code and the code to get SDA out of low state
 * are based on Silicon Laboratories's F02x_SMBus_EEPROM.c
 *
 * This sample demonstrates the use of DS1307 on SMB interface
 */
#include <C8051F020.h>
#include <stdio.h>
#ifndef SYSCLK
#define SYSCLK 22118400
#endif
#ifndef BAUDRATE
#define BAUDRATE 115200
#endif
#define I2CCLK 80000 // 80k
#define  SMB_BUS_ERROR  0x00           // (all modes) BUS ERROR
#define  SMB_START      0x08           // (MT & MR) START transmitted
#define  SMB_RP_START   0x10           // (MT & MR) repeated START
#define  SMB_MTADDACK   0x18           // (MT) Slave address + W transmitted;
                                       //    ACK received
#define  SMB_MTADDNACK  0x20           // (MT) Slave address + W transmitted;
                                       //    NACK received
#define  SMB_MTDBACK    0x28           // (MT) data byte transmitted;
                                       //    ACK rec'vd
#define  SMB_MTDBNACK   0x30           // (MT) data byte transmitted;
                                       //    NACK rec'vd
#define  SMB_MTARBLOST  0x38           // (MT) arbitration lost
#define  SMB_MRADDACK   0x40           // (MR) Slave address + R transmitted;
                                       //    ACK received
#define  SMB_MRADDNACK  0x48           // (MR) Slave address + R transmitted;
                                       //    NACK received
#define  SMB_MRDBACK    0x50           // (MR) data byte rec'vd;
                                       //    ACK transmitted
#define  SMB_MRDBNACK   0x58           // (MR) data byte rec'vd;
                                       //    NACK transmitted
#define S_OVER 4
#define R_OVER 3
#define SMB_FAIL 1
unsigned char smb_buf[20];             // SMB transmit/receive buffer
unsigned char smb_len;                 // transfer length
volatile int result;

sbit SCL = P0 ^ 3;
sbit SDA = P0 ^ 2;

void SYSCLK_Init()
{
    int i;
    OSCXCN = 0x67;  // enabled external crystal
    for (i=0; i < 256; i++);
    while (!(OSCXCN & 0x80));  // wait for stable
    OSCICN = 0x88;  // use external crystal
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

void Delay(int k)
{
    int i;
    for (i = 0; i < k; ++i);
}

void Timer0_ms (int ms)
{
    int i;
    TCON &= ~0x30;
    TMOD &= ~0x0f;
    TMOD |= 0x01;
    CKCON |= 0x08;
    for (i = 0; i < ms; ++i) {
        TR0 = 0;
        TH0 = (-SYSCLK/1000) >> 8;
        TL0 = -SYSCLK/1000;
        TR0 = 1;
        while (TF0 == 0);
        TF0 = 0;
    }
}


void PORT_Init()
{
    XBR0 = 0x05;     // enable i2c and uart0
    XBR2 = 0x40;     // enable xbr
    P0MDOUT |= 0x01; // TX as push-pull output
}

void I2C_Init()
{
    SMB0CN = 0x07;   // Assert Acknowledge low (AA bit = 1b);
    // Enable SMBus Free timeout detect;
    // Enable SCL low timeout detect
    SMB0CR = 257 - (SYSCLK / (2 * I2CCLK));
    SMB0CN |= 0x40;                     // Enable SMBus;
    STO = 0;
}

void SMBUS_ISR (void) interrupt 7
{
    bit FAIL = 0;            // 记录异常情况
    static unsigned char i;  // 缓存指针记录

    switch (SMB0STA)         // 状态寄存器
    {
        case SMB_START:      // 主设备起始位已发送
        case SMB_RP_START:   // 主设备重复起始位已发送
            SMB0DAT = smb_buf[0];  // 地址+读写位
            STA = 0;               // 清除起始位标志
            i = 0;                 // 指针清零jj
            break;
        case SMB_MTADDACK:         // 主设备发送，从设备确认地址
            SMB0DAT = smb_buf[1];  // 发送后续数据
            break;
        case SMB_MTDBACK:          // 主设备发送，从设备确认数据
            if (i < smb_len)       // 是否有更多数据？
            {
                SMB0DAT = smb_buf[2 + i]; // 传递下一个字节
                i++;               // 指针加 1
            } else {
                result = S_OVER;   // 结束状态
                STO = 1;           // 设置停止位标志
            }
            break;
        case SMB_MRADDACK:     // 主设备接收，从设备确认地址
            if (smb_len == 1)  // 只要接收一个数据
                AA = 0;        // 接下来将回复 NACK
            else
                AA = 1;        // 接下来将回复 ACK
            break;
        case SMB_MRDBACK:      // 主设备接收，已发送 ACK
            if ( i < smb_len)  // 如果还有更多数据
            {
                smb_buf[i + 1] = SMB0DAT;  // 保存已接到数据
                i++;                       // 指针加 1
                AA = 1;                    // 准备发送 ACK
            }
            if (i >= smb_len)  // 当前数据是最后字节
                AA = 0;        // 准备发送 NACK

            break;

        case SMB_MRDBNACK:     // 主设备接受，NACK 已发送
            smb_buf[i + 1] = SMB0DAT;      // 保存已接到数据
            STO = 1;           // 准备发送结束位
            AA = 1;            // AA 状态复位
            result = R_OVER;   // 结束状态
            break;
        case SMB_MTADDNACK:    // 主设备写，从设备未确认地址
        case SMB_MTDBNACK:     // 从设备未确认数据
        case SMB_MTARBLOST:    // 主设备竞争总线失败
            // Master Receiver: Slave address + READ transmitted.  NACK received.
        case SMB_MRADDNACK:    // 主设备读，从设备未确认地址
            FAIL = 1;          // 设置失败标志
            break;
        default:
            FAIL = 1;

            break;
    }
    if (FAIL)                   // 传输失败
    {
        SMB0CN &= ~0x40;        // 复位 SMB 设备
        SMB0CN |= 0x40;
        STA = 0;
        STO = 0;
        AA = 0;
        result = SMB_FAIL;      // 结束状态
        FAIL = 0;
    }
    SI = 0;                     // 清除中断标志
}

void SMB_Transmit(unsigned char addr, unsigned char len)
{
    result = 0;         // 清除结束状态
    smb_buf[0] = 0xD0;  // 从设备地址 + 写入标志
    smb_buf[1] = addr;  // DS1307 寄存器地址
    smb_len = len;      // 写入数据长度
    STO = 0;            // 准备发送起始位
    STA = 1;
    while (result == 0);// 等待发送结束
    Delay(100);
}

void SMB_Receive(unsigned char len)
{
    result = 0;         // 清除结束状态
    smb_buf[0] = 0xD1;  // 从设备地址 + 读出标志
    smb_len = len;      // 读取的长度
    STO = 0;            // 准备发送起始位
    STA = 1;
    while (result == 0);// 等待发送结束
    Delay(100);
}

void main()
{
    int i;
    unsigned char c;
    unsigned char buf[16];
    unsigned char d;
    WDTCN=0xDE;
    WDTCN=0xAD;
    SYSCLK_Init();
    PORT_Init();
    UART0_Init();
    TI0 = 1;
    I2C_Init();
    SI = 0;                // 清除中断标志
    EIE1 |= 0x02;          // 允许 SMB 的中断
    EA = 1;                // 允许全局中断

    SMB_Transmit(0, 0);    // 写入 DS1307 操作地址
    SMB_Receive(1);        // 读取第一个字节
    if (smb_buf[1] & 0x80) {    // 如果 CH 为 1
        unsigned char b = smb_buf[1];
        smb_buf[2] = b & 0x7F;  // 设置要写入的数据
        SMB_Transmit(0, 1);     // CH 置 0
    }
    while(1) {
        /*SMB_Transmit(0, 0);
        SMB_Receive(8);         // 接收时间数据
        Timer0_ms(500);
        for (i = 0; i < 8; ++i) { // 显示到串口
            printf("%02bx ", smb_buf[i + 1]);
        }
        printf("\r\n"); */
	    do {
	    	c = getchar();
		} while ((c == ' ') || (c == '\r') || (c == '\n'));
        if (c == 'd') {
            SMB_Transmit(0, 0);
            SMB_Receive(8);
            sprintf(buf, "20%02bx-%02bx-%02bx ", smb_buf[7], smb_buf[6], smb_buf[5]);
            printf("\r\n%s", buf);
            sprintf(buf, "%02bx:%02bx:%02bx\r\n", smb_buf[3], smb_buf[2], smb_buf[1]);
            printf("%s", buf);
        }
        if (c == 'w') {
            for (i = 0; i < 7; ++i) {
                scanf("%bx", &d);
                smb_buf[8 - i] = d;
            }
            printf("\r\n");
            for (i = 0; i < 7; ++i) printf("%2bx ", smb_buf[i + 2]);
            printf("\r\n");
            SMB_Transmit(0, 7);
        }
    }
}

