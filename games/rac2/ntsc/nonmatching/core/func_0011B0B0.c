#include "common.h"

extern s32 func_0011F5E0(void);

extern s32 func_0011F628(void);

void func_0011B0B0(void) {
    s32 enabled;

    enabled = func_0011F5E0();
    *(volatile u32 *) 0x1000F000 = 4;
    __sync_synchronize();
    if (enabled != 0) {
        func_0011F628();
    }
    while ((*(volatile u32 *) 0x1000F000 & 4) == 0) {
    }
    enabled = func_0011F5E0();
    *(volatile u32 *) 0x1000F000 = 4;
    __sync_synchronize();
    if (enabled != 0) {
        func_0011F628();
    }
}
/* Near miss: everything but the two barriers.
   Compiled C: save the interrupt state, write 4 to D_ENABLER, `sync`, restore
   the state, poll D_ENABLER bit 2, do it again and tail-call the restore. The
   two `sync` instructions cannot come from C here -- `__sync_synchronize()`
   compiles into a call to an undefined external, and none of the ~2900
   functions already matched in src/ contains a 0x0000000F. Removing the two
   barriers leaves the rest of the function in place. The original almost
   certainly had an `asm` barrier, so this may belong as ASM_FUNC rather than a
   near miss; that is a maintainer's call. */
