#include "include_asm.h"

/* Small-data file: this body reaches its global through $gp, so it only
 * reproduces at -G8. */

extern int D_L15_001B187C __attribute__((sda));

extern void func_L15_00437B68(int, int);

void func_L15_00438238(int a0) {
    if (D_L15_001B187C != 0) {
        func_L15_00437B68(a0, 1);
    }
}
