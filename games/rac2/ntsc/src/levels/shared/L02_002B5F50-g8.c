#include "include_asm.h"

extern unsigned int D_L02_001A8F00;

/* Report whether the resident flag word is still clear. */
int func_L02_002F1658(void) {
    return D_L02_001A8F00 == 0;
}
