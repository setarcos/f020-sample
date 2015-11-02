#include <C8051F020.h>
#include <stdio.h>
#include "pku.h"

#define CMD_WR  0x80
#define CMD_RD  0x40
#define CMD_INT 0x00
#define BASE_ADD 0x200

void PORT_Init()
{
    XBR0 = 0x04;     // uart0 enabled in xbr
    EMI0CF = 0x3F;   // EXIF on P47, non-multiplexed mode
    XBR1 = 0x04;     // enable int0 in xbr
    XBR2 = 0x40;     // enable xbr
    P0MDOUT |= 0x01; // set tx0 as push-pull
    P74OUT = 0xFF;   // all push-pull
}

__bit write_phase;
__bit read_phase;
__bit int_phase;
__bit no_transmit;
__bit data_phase;

unsigned char addr;
unsigned char data;

unsigned char SendIt(unsigned char it)
{
    if (no_transmit) {
        no_transmit = 0;
        SBUF0 = it;
       // TI0 = 1;
        return 0;
    }
    return 1;
}

__xdata __at (BASE_ADD) unsigned char regs[64];

void INT0_ISR (void) __interrupt (0)
{
    if (int_phase) return; // in case interrupt occur too often
    if (SendIt(CMD_INT))
        int_phase = 1;
}

void UART0_ISR (void) __interrupt (4)
{
    unsigned char c;
    if (RI0 == 1) {
        RI0 = 0;
        c = SBUF0;
        if (write_phase) {
            if (SendIt(CMD_WR)) {
                data_phase = 1;
                data = CMD_WR;
            }
            regs[addr] = c;
            write_phase = 0;
        } else {
            if ((c & 0xC0) == CMD_WR) {
                addr = (c & 0x1F);
                write_phase = 1;
            }
            if ((c & 0xC0) == CMD_RD) {
                addr = (c & 0x1F);
                read_phase = 1;
                if (SendIt(CMD_RD)) {
                    data = CMD_RD;
                    data_phase = 1;
                }
            }
        }
    }
    if (TI0 == 1) {
        TI0 = 0;
        if (data_phase) { // after int_phase
            SBUF0 = data;
            data_phase = 0;
        }
        if (read_phase) {
            SBUF0 = regs[addr];
            read_phase = 0;
        }
        if (int_phase) {
            SBUF0 = CMD_INT;
            int_phase = 0;
        }
        no_transmit = 1;
    }
}

void main()
{
    WDTCN=0XDE;
    WDTCN=0XAD;
    SYSCLK_Init();
    PORT_Init();
    UART0_Init();
    write_phase = 0;
    read_phase = 0;
    data_phase = 0;
    int_phase = 0;
    no_transmit = 1;
    IE0 = 0;  // hardware clear int0 flag
    IT0 = 1;  // falling edge trigger
    EX0 = 1;  // enable int0
    EA = 1;
    ES0 = 1;    // enable UART0 interrupt
    while(1);
}

