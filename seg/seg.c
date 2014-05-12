/* Sample code for using the 7-segment LED display */
#include <C8051F020.h>
#include "pku.h"

__xdata __at (0x8000) unsigned char seg;
__xdata __at (0x8001) unsigned char cs;

__code const unsigned char segs[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E};
__code const unsigned char css[] = {0x7, 0xB, 0xD, 0xE};

void main(void)
{
    unsigned int k;
    WDTCN = 0xde;
    WDTCN = 0xad; // Disable watchdog
    EMIF_Low();
    k = 0;
    while (1) {
        unsigned char i;
        int j;
        j = k / 64;
        for (i = 0; i < 4; ++i) {
            seg = segs[j % 10];
            cs = css[i];
            j /= 10;
            Delay(1000);
        }
        k++;
    }
}
