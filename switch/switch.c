#include <C8051F020.h>
#include "pku.h"

__xdata __at (0x8000) unsigned char seg;
__xdata __at (0x8001) unsigned char cs;
volatile __xdata __at (0x8007) unsigned char key;

__code const unsigned char segs[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E};
__code const unsigned char css[] = {0x7, 0xB, 0xD, 0xE};

void main(void)
{
    WDTCN = 0xde;
    WDTCN = 0xad; // Disable watchdog
    EMIF_Low();
    while (1) {
        unsigned char i;
        unsigned char j;
        j = key;
        for (i = 0; i < 2; ++i) {
            seg = segs[j & 0xF];
            cs = css[i];
            j = j >> 4;
            Delay(500);
            seg = 0xFF;
            cs = 0xF;
        }
    }
}
