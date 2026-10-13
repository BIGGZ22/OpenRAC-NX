#include "include_asm.h"

extern int D_001ADAF8 __attribute__((sda));

int func_00336EF8(unsigned int index) {
    if (index >= 4u) return 0;
    return ((int *)&D_001ADAF8)[index];
}
