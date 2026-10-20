/* 0 of 27 places match - last line: 00_aranos_tutorial: 132 bytes, expected 136
   Books a1 into the slot of the identity's index (func_L00_002D3FB0(a0), a level-local
   copy), then clamps: the per-level entry's 0x8E field is the ceiling, the slot keeps it
   and the surplus is returned.  The file's own `Entry_262570` struct (0xE0 bytes, f8E at
   0x8E) and the D_L00_00139568 / D_L00_00262570 / D_L00_00139688 symbols are exactly
   right, and with a 32-bit prototype for func_L00_002D3FB0 the candidate is 132 bytes
   with every instruction matching except one register copy: retail loads the field into
   $v0 and copies it to $a1 in the `beqz` delay slot (`daddu $a1,$v0,$zero`) and keeps the
   running sum in $a2, while this cc1 loads the field straight into the register it uses
   ($v1) and keeps the sum in $v0, saving that copy.
   Note for the next attempt: the file's `unsigned long func_L00_002D3FB0(unsigned long)`
   adds `dsll32`/`dsra32` in the caller (140 bytes); retail's caller has neither, so the
   retail prototype is 32-bit.  The helper's own body (`return param_1;`) compiles the same
   either way.
   Tried and identical or worse: `v <= ceiling` (136 bytes, 18 words), the `||` chain
   (128 bytes), an `unsigned short` ceiling, and the inverted `if (...) { ... } return 0;`. */

#include "include_asm.h"

extern int func_L00_002D3FB0(int);
extern unsigned char D_L00_00139568[];
extern int D_L00_00139688[];

int func_L00_002D3FB8(int a0, int a1)
{
    int idx = func_L00_002D3FB0(a0);
    int *slot = &D_L00_00139688[idx];
    Entry_262570 *e;
    int v = *slot + a1;
    int ceiling;

    *slot = v;
    e = &D_L00_00262570[D_L00_00139568[idx]];
    ceiling = e->f8E;
    if (ceiling == 0)
        return 0;
    if (v - ceiling <= 0)
        return 0;
    *slot = ceiling;
    return v - ceiling;
}
