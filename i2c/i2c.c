/* The ISR code and the code to get SDA out of low state
 * are based on Silicon Laboratories's F02x_SMBus_EEPROM.c
 *
 * This sample demonstrates the use of DS1307 on SMB interface
 */
#include <c8051f020.h>
#include <stdio.h>
#include "pku.h"
#define I2CCLK 80000 // 100k
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
unsigned char smb_buf[20];             // SMB transmit/receive buffer
unsigned char smb_len;                 // transfer length
volatile int result;

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

void SMBUS_ISR (void) __interrupt (7)
{
    __bit FAIL = 0;                     // Used by the ISR to flag failed transfers
    static unsigned char i;             // Used by the ISR to index the buffer

    // Status code for the SMBus (SMB0STA register)
    switch (SMB0STA)
    {
        // Master Transmitter/Receiver: START condition transmitted.
        case SMB_START:
        case SMB_RP_START:
            SMB0DAT = smb_buf[0];          // Address | RW_BIT
            STA = 0;                      // Manually clear STA bit
            i = 0;                        // Reset data byte counter
            break;

            // Master Transmitter: Slave address + WRITE transmitted.  ACK received.
        case SMB_MTADDACK:
            SMB0DAT = smb_buf[1];
            break;

            // Master Transmitter: Slave address + WRITE transmitted.  NACK received.
        case SMB_MTADDNACK:
            FAIL = 1;                  // Indicate failed transfer
            break;

            // Master Transmitter: Data byte transmitted.  ACK received.
            // For a WRITE: each data word should be sent
        case SMB_MTDBACK:
            if (i < smb_len)      // Is there data to send?
            {
                // Send data byte
                SMB0DAT = smb_buf[2 + i];
                i++;
            } else {
                result = 4;
                STO = 1;                 // Set STO to terminte transfer
            }
            break;

            // Master Transmitter: Data byte transmitted.  NACK received.
        case SMB_MTDBNACK:
            FAIL = 1;                  // Indicate failed transfer
            break;

            // Master Transmitter: Arbitration lost.
        case SMB_MTARBLOST:
            FAIL = 1;                     // Indicate failed transfer
            break;

            // Master Receiver: Slave address + READ transmitted.  NACK received.
        case SMB_MRADDNACK:
            FAIL = 1;                  // Indicate failed transfer
            break;

            // Master Receiver: Slave address + READ transmitted.  ACK received.
        case SMB_MRADDACK:
            if (smb_len == 1)
                AA = 0;        // Send NACK
            else
                AA = 1;        // Send ACK
            break;
            // Master Receiver: Data byte received.  ACK transmitted.
        case SMB_MRDBACK:
            if ( i < smb_len)       // Is there any data remaining?
            {
                smb_buf[i + 1] = SMB0DAT;       // Store received byte
                i++;                       // Increment number of bytes received
                AA = 1;                    // Send ACK (may be cleared later
                // in the code)
            }
            if (i >= smb_len)        // This is the last byte
            {
                AA = 0;                    // Send NACK to indicate last byte
            }

            break;

            // Master Receiver: Data byte received.  NACK transmitted.
            // Read operation has completed.  Read data register and send STOP.
        case SMB_MRDBNACK:
            smb_buf[i + 1] = SMB0DAT;      // Store received byte
            STO = 1;
            AA = 1;                       // Set AA for next transfer
            result = 2;
            break;

            // All other status codes invalid.  Reset communication.
        default:
            FAIL = 1;

            break;
    }
    if (FAIL)                           // If the transfer failed,
    {
        SMB0CN &= ~0x40;                 // Reset communication
        SMB0CN |= 0x40;
        STA = 0;
        STO = 0;
        AA = 0;
        result = 1;
        FAIL = 0;
    }
    SI = 0;                             // Clear interrupt flag
}

void SMB_Transmit(unsigned char addr, unsigned char len)
{
    result = 0;
    smb_buf[0] = 0xD0;
    smb_buf[1] = addr;
    smb_len = len;
    STO = 0;
    STA = 1;
    while (result == 0);
    Delay(100);
}

void SMB_Receive(unsigned char len)
{
    result = 0;
    smb_buf[0] = 0xD1;
    smb_len = len;
    STO = 0;
    STA = 1;
    while (result == 0);
    Delay(100);
}

void main()
{
    int i;
    WDTCN=0XDE;
    WDTCN=0XAD;
    SYSCLK_Init();

    while (!(P0_2))
    {
        // Provide clock pulses to allow the slave to advance out
        // of its current state. This will allow it to release SDA.
        XBR2 = 0x40;                     // Enable Crossbar
        P0_3 = 0;                         // Drive the clock low
        Delay(1200);        // Hold the clock low
        P0_3 = 1;                         // Release the clock
        while(!P0_3);                     // Wait for open-drain
        // clock output to rise
        Delay(60);         // Hold the clock high
        XBR2 = 0x00;                     // Disable Crossbar
        Delay(10);
    }
    PORT_Init();
    UART0_Init();
    I2C_Init();
    SI = 0;                           // Clear Interrupte flag
    EIE1 |= 0x02;                     // Enable the SMBus interrupt
    EA = 1;                           // Global interrupt enable

    SMB_Transmit(0, 0); // Set registor pointer
    SMB_Receive(1);  // Read first byte of DS1307;
    if (smb_buf[1] & 0x80) {
        unsigned char b = smb_buf[1];
        SMB_Transmit(0, 0);
        smb_buf[2] = b & 0x7F;
        SMB_Transmit(0, 1); // Start DS1307
    }
    while(1) {
        SMB_Transmit(0, 0);
        SMB_Receive(14);
        for (i = 0; i < 0x7fff; ++i)
            Delay(0xFFFF);
        for (i = 0; i < 14; ++i) {
            printf("%02x ", smb_buf[i + 1]);
        }
        printf("\r\n");
    }
}

