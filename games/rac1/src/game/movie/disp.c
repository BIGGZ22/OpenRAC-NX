#include "common.h"
#include "structs.h"

/*
 * movie/disp.cpp in the original source; text 0x23C5E0-0x23C9C0.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

INCLUDE_ASM("asm/nonmatchings/text", func_0023C5E0); /* setImageTag */
ASM_FUNC("asm/handwritten/text", func_0023C7A8); /* vblankHandler */
INCLUDE_ASM("asm/nonmatchings/text", func_0023C910); /* handler_endimage */
extern volatile int D_001612E0 NOT_SDA;   /* display-active flag, shared with the vblank handler */
extern volatile int D_001612E4 NOT_SDA;
extern int func_00122598(int);

/* startDisplay(int) -- waits until the vertical-sync field differs from the given one, then
 * marks the display active. */
void func_0023C960(int field) {
    while (func_00122598(0) == field) {
    }
    D_001612E0 = 1;
    D_001612E4 = 0;
}
/* endDisplay(void) -- clears the display-active flag. */
void func_0023C9B0(void) {
    D_001612E0 = 0;
}
