/* from src/core/0011E4E0.c: the catalogue row is 232 bytes but the function is 136.
 *
 * The row's first 96 bytes (0x11E740..0x11E79C) are not this function: they are the
 * linker's padding between two functions, an `addiu $sp,$sp,N ; nop` chain with no
 * `jr $ra` (twelve pairs, N = 0xC0, 0xD0, 0xD0, 0x10, 0xD0, 0x10, 0xD0, 0x10, 0xE0,
 * 0x10, 0xE0, 0x10), the same shape docs/COMPILER-NOTES.md calls "catalogue rows that
 * are not functions". They sit right after the epilogue of func_0011E4E0 (sceRead, the
 * 608 bytes of C in this file), whose last two words are `jr $ra ; addiu $sp,$sp,0xE0`
 * at 0x11E738.
 *
 * The real body runs from 0x11E7A0 to 0x11E828 (136 bytes: `lui $s1,0x14`,
 * `addiu $sp,$sp,-0x30`, and the `jr $ra ; addiu $sp,$sp,0x30` at 0x11E820). The boot's
 * only call into this code is the `jal` at 0x290D60, and it calls 0x11E7A0; no `jal` in
 * the boot targets 0x11E740.
 *
 * The body binds the RPC 0x80000003 on the client at D_00140140 (sceSifBindRpc,
 * func_0011D450), waits for the client's word at +0x24 (D_00140164) to leave zero with
 * a 0x100000-iteration delay between the polls, clears D_00134744 and returns 0 (-1 when
 * the bind fails).
 *
 * The C below reproduces those 136 bytes exactly: with it in src/core/0011E4E0.c, compiled
 * in this unit with the SDK compiler (`sdk -O2 -G2 -ffunction-sections`,
 * config/cflags.txt), it is 136 bytes and tools/build.py's own Placement.derive() finds
 * no difference at 0x11E7A0, while tools/check.py func_0011E740 prints "136 bytes,
 * expected 232" because the row starts 96 bytes early. C cannot emit the padding, so the
 * row as catalogued is unreachable from src/.
 *
 * What would unblock it: a tools/levels.py split that starts the row at 0x11E7A0
 * (136 bytes) and gives the 96 padding bytes to the previous function or to a filler
 * row, as the catalogue already does elsewhere for remnants.
 */
#include "common.h"

extern int func_0011D450(void *, int, int);

extern char D_00140140[];

extern int D_00134744;

s32 func_0011E740(void)
{
    s32 i;

    for (;;) {
        if (func_0011D450(D_00140140, 0x80000003, 0) < 0) {
            return -1;
        }
        if (((s32 *)D_00140140)[9] != 0) {
            D_00134744 = 0;
            break;
        }
        for (i = 0x100000; i != -1; i--) {
        }
    }
    return 0;
}
