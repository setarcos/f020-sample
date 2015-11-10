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
    bit FAIL = 0;            // ��¼�쳣���
    static unsigned char i;  // ����ָ���¼

    switch (SMB0STA)         // ״̬�Ĵ���
    {
        case SMB_START:      // ���豸��ʼλ�ѷ���
        case SMB_RP_START:   // ���豸�ظ���ʼλ�ѷ���
            SMB0DAT = smb_buf[0];  // ��ַ+��дλ
            STA = 0;               // �����ʼλ��־
            i = 0;                 // ָ������jj
            break;
        case SMB_MTADDACK:         // ���豸���ͣ����豸ȷ�ϵ�ַ
            SMB0DAT = smb_buf[1];  // ���ͺ�������
            break;
        case SMB_MTDBACK:          // ���豸���ͣ����豸ȷ������
            if (i < smb_len)       // �Ƿ��и������ݣ�
            {
                SMB0DAT = smb_buf[2 + i]; // ������һ���ֽ�
                i++;               // ָ��� 1
            } else {
                result = S_OVER;   // ����״̬
                STO = 1;           // ����ֹͣλ��־
            }
            break;
        case SMB_MRADDACK:     // ���豸���գ����豸ȷ�ϵ�ַ
            if (smb_len == 1)  // ֻҪ����һ������
                AA = 0;        // ���������ظ� NACK
            else
                AA = 1;        // ���������ظ� ACK
            break;
        case SMB_MRDBACK:      // ���豸���գ��ѷ��� ACK
            if ( i < smb_len)  // ������и�������
            {
                smb_buf[i + 1] = SMB0DAT;  // �����ѽӵ�����
                i++;                       // ָ��� 1
                AA = 1;                    // ׼������ ACK
            }
            if (i >= smb_len)  // ��ǰ����������ֽ�
                AA = 0;        // ׼������ NACK

            break;

        case SMB_MRDBNACK:     // ���豸���ܣ�NACK �ѷ���
            smb_buf[i + 1] = SMB0DAT;      // �����ѽӵ�����
            STO = 1;           // ׼�����ͽ���λ
            AA = 1;            // AA ״̬��λ
            result = R_OVER;   // ����״̬
            break;
        case SMB_MTADDNACK:    // ���豸д�����豸δȷ�ϵ�ַ
        case SMB_MTDBNACK:     // ���豸δȷ������
        case SMB_MTARBLOST:    // ���豸��������ʧ��
            // Master Receiver: Slave address + READ transmitted.  NACK received.
        case SMB_MRADDNACK:    // ���豸�������豸δȷ�ϵ�ַ
            FAIL = 1;          // ����ʧ�ܱ�־
            break;
        default:
            FAIL = 1;

            break;
    }
    if (FAIL)                   // ����ʧ��
    {
        SMB0CN &= ~0x40;        // ��λ SMB �豸
        SMB0CN |= 0x40;
        STA = 0;
        STO = 0;
        AA = 0;
        result = SMB_FAIL;      // ����״̬
        FAIL = 0;
    }
    SI = 0;                     // ����жϱ�־
}

void SMB_Transmit(unsigned char addr, unsigned char len)
{
    result = 0;         // �������״̬
    smb_buf[0] = 0xD0;  // ���豸��ַ + д���־
    smb_buf[1] = addr;  // DS1307 �Ĵ�����ַ
    smb_len = len;      // д�����ݳ���
    STO = 0;            // ׼��������ʼλ
    STA = 1;
    while (result == 0);// �ȴ����ͽ���
    Delay(100);
}

void SMB_Receive(unsigned char len)
{
    result = 0;         // �������״̬
    smb_buf[0] = 0xD1;  // ���豸��ַ + ������־
    smb_len = len;      // ��ȡ�ĳ���
    STO = 0;            // ׼��������ʼλ
    STA = 1;
    while (result == 0);// �ȴ����ͽ���
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
    SI = 0;                // ����жϱ�־
    EIE1 |= 0x02;          // ���� SMB ���ж�
    EA = 1;                // ����ȫ���ж�

    SMB_Transmit(0, 0);    // д�� DS1307 ������ַ
    SMB_Receive(1);        // ��ȡ��һ���ֽ�
    if (smb_buf[1] & 0x80) {    // ��� CH Ϊ 1
        unsigned char b = smb_buf[1];
        smb_buf[2] = b & 0x7F;  // ����Ҫд�������
        SMB_Transmit(0, 1);     // CH �� 0
    }
    while(1) {
        /*SMB_Transmit(0, 0);
        SMB_Receive(8);         // ����ʱ������
        Timer0_ms(500);
        for (i = 0; i < 8; ++i) { // ��ʾ������
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

