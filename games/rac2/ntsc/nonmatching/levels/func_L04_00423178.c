/* func_L04_00423178, 224 bytes, 7 placements.  ABANDON - the address-CSE wall (260 bytes
   nearest).
   Armed only when D_001A8F00 == 2, D_001A7A40 is set and the record block at 0x001BD380 has
   a positive count at +0x44: walk that block's +0x18C pointer array and, for every record
   whose +0xAA short equals the argument's +0xAA while D_001B1DE1 is clear, call
   func_3325E0(record, 0, D_001B1DE0) and publish 4.0f into D_001B1E00, D_001B1E04 and
   D_001B1E08.
   Retail rematerialises every absolute address site by site - `lui $v1,0x1B` /
   `lw $v1,-0x7100($v1)` for the first guard, `lui $v0,0x1A` / `lbu $v0,0x7A40($v0)`,
   `lui $v0,0x1B` / `lbu $v0,0x1DE1($v0)`, and three separate `lui $at,0x1B` /
   `swc1 $f20,0x1E00/0x1E04/0x1E08($at)` stores, with only the 0x001BD380 block held in a
   register (`lui $v1,0x1C` / `addiu $v1,$v1,-0x2C80`).  This cc1 CSEs the addresses instead
   and the frame grows from 0x40 to 0x60 (extra $s4-$s7/$fp saves) or keeps the count in a
   register.  Spellings measured: plain externs with an index walk (272), a moving pointer
   (276), a named count (260), and the `nosda` attribute on every symbol (272 with the same
   oversized frame).  The count is re-read at the loop bottom in retail (`lh $v0,0x44($s2)`
   in both annulled `bnel` delay slots and in the loop test's), which the source reproduces by
   writing the bound as `*(s16 *)(s2 + 0x44)` in the loop head. */

#include "include_asm.h"
#include "common.h"

extern s32 D_001A8F00;

extern u8 D_001A7A40;

extern u8 D_001B1DE1;

extern char D_001B1DE0[];

extern f32 D_001B1E00;

extern f32 D_001B1E04;

extern f32 D_001B1E08;

extern char D_001BD380[];

extern void func_3325E0(char *a0, s32 a1, char *a2);

void func_L04_00423178(char *a0)
{
    char *s2 = D_001BD380;
    s32 i;

    if (D_001A8F00 != 2)
        return;
    if (D_001A7A40 == 0)
        return;
    if (*(s16 *)(s2 + 0x44) <= 0)
        return;
    for (i = 0; i < *(s16 *)(s2 + 0x44); i++) {
        char *p = *(char **)(s2 + 0x18C + i * 4);

        if (*(s16 *)(p + 0xAA) != *(s16 *)(a0 + 0xAA))
            continue;
        if (D_001B1DE1 != 0)
            continue;
        func_3325E0(p, 0, D_001B1DE0);
        D_001B1E00 = 4.0f;
        D_001B1E04 = 4.0f;
        D_001B1E08 = 4.0f;
    }
}
