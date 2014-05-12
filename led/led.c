#include <C8051F020.h>
#include "pku.h"

void main(void)
{
    WDTCN = 0xde;
    WDTCN = 0xad; // disable watchdog
    P74OUT = 0x80; // push-pull
    while (1) {
        P7 = 0x40;
        Delay(20000);
        P7 = 0;
        Delay(20000);
    }
}
