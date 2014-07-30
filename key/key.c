#include <C8051F020.h>
#include "pku.h"

__xdata __at (0x8000) unsigned char seg;
__xdata __at (0x8001) unsigned char cs;

__code const unsigned char segs[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E};
__code const unsigned char css[] = {0x7, 0xB, 0xD, 0xE};
__code const unsigned char dec[] = {0, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0};
__code const unsigned char trans[] = {0xC, 9, 5, 1, 0xD, 0, 6, 2, 0xE, 0xA, 7, 3, 0xF, 0xB, 8, 4};

void main(void)
{
    WDTCN = 0xde;
    WDTCN = 0xad; // Disable watchdog
    EMIF_Low();

    cs = 7;
    while (1) {
        unsigned char i;
        unsigned char key;
        P4 = 0x0F;
        Delay(100);
        i = ~P4 & 0x0F;
        if (i == 0) continue; // no key
        key = dec[i] << 2;
        Delay(1000);          // debounce
        i = ~P4 & 0x0F;
        if (i == 0) continue; // no key;
        P4 = 0xF0;
        Delay(100);
        i = ~P4;
        i >>= 4;
        key = key + dec[i];
        seg = segs[trans[key & 0xF]]; // show on display
        Delay(1000);
    }
}
