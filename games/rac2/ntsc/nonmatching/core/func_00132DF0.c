/* from src/core/00131780-g8.c: 716 bytes, 736 expected; the callee-saved
 * allocation is the gap. This cc1 keeps the loop constants (0x100, 1) and the
 * error-string base in $s6/$fp/$s7 and spills cmd, data and arg3 to
 * 0x10(sp)/0x14(sp)/0x18(sp) (frame 0x80), where retail keeps the three
 * arguments in $s6/$s7/$fp, materialises the constants in temporaries and
 * needs 0x70. Five more instructions are missing elsewhere.
 *
 * snd_QueueCommandToRing (989snd, rac1-decomp src/core/989snd.c func_0012E800
 * at delta +0x45F0; rac1 keeps it as INCLUDE_ASM too, so there is no upstream
 * C). Append one command to the double-buffered ring selected by
 * r1_D_001A74C0, pumping and re-testing while the slot is busy or the free
 * space is short; a zero-size command on a clean ring goes out through the
 * command RPC (g_sndRpcClientCmd / g_sndCmdRpcRecv) instead of the ring.
 *
 * $gp recipe, measured here (the unit's other near miss, func_001330D0, says
 * an sda array still comes out absolute): the address does become
 * `addiu $x,$gp,-0x7B50` once the extern array is declared with a SIZE the
 * -G8 limit accepts - `extern int *x[2] __attribute__((sda));` - because an
 * incomplete array type is never classified as small data. Scalars
 * (r1_D_001A74C0/C4, r1_D_001A7480, r1_D_001A74F8) already load
 * gp-relative with the same attribute. Everything else in this file agrees:
 * the two spin/fast paths, the alignment guard `(len & 3)`,
 * `len = size + 8 - (len % 4)`, the scan loop's `continue`/`break` shape, the
 * 4-byte header at `table[idx] - (limits[idx] - 0x1000)`, the byte copy and
 * the two 16-byte ring-entry stores before func_001330D0. */

#include "common.h"

extern int r1_D_001A74C4 __attribute__((sda));

extern int r1_D_001A7480 __attribute__((sda));

extern int r1_D_001A74F8 __attribute__((sda));

extern int r1_D_001A74C0 __attribute__((sda));

extern int *r1_D_001A74A0[2] __attribute__((sda));

extern int r1_D_001A74A8[2] __attribute__((sda));

extern u8 *r1_D_001A74B0[2] __attribute__((sda));

extern int r1_D_001A7040[];

extern int r1_D_001A7080[];

extern char r1_D_001A7578[];

extern char r1_D_001A78D0[];

extern char r1_D_0013BFF8[];

extern void func_001322B8(void *, int);

extern int func_0011D810(void *);

extern int func_0011D620(void *, int, int, void *, int, void *, int, void *, int);

extern void func_00115D48(char *, ...);

extern void func_0011AEA0(int);

extern void func_001330D0(void);

typedef struct {
    s32 field0;
    s32 pad;
    s64 field8;
} SndRingEntry;

void func_00132DF0(s32 cmd, s32 size, u8 *data, s32 arg3, s64 arg4) {
    s32 len;
    s32 tries;
    s32 flipped;
    s32 i;
    u8 *dst;
    SndRingEntry *entry;

    len = size + 4;
    if (r1_D_001A74C4 == 0 && r1_D_001A7480 == 0 && size == 0 && arg3 == 0) {
        func_001322B8(r1_D_001A7080, 1);
        while (func_0011D810(r1_D_001A7040) != 0) {
            if (r1_D_001A74F8 == 0) {
                func_00115D48(r1_D_001A7578);
            }
            func_00132028();
            func_0011AEA0(0);
        }
        func_0011D620(r1_D_001A7040, cmd, 1, 0, 0, r1_D_001A7080, 0xC, 0, 0);
        return;
    }
    if ((len & 3) != 0) {
        len = size + 8 - (len % 4);
    }
    tries = 0;
    flipped = 0;
    if (*r1_D_001A74A0[r1_D_001A74C0] == 0x100) {
        while (1) {
            if (r1_D_001A74C4 != 0) {
                r1_D_001A74C4 = 0;
                flipped = 1;
            }
            func_00132028();
            if (tries != 1 && r1_D_001A74F8 == 0) {
                func_00115D48(r1_D_0013BFF8, r1_D_001A74C0,
                              *r1_D_001A74A0[r1_D_001A74C0]);
            }
            if (*r1_D_001A74A0[r1_D_001A74C0] == 0x100) {
                continue;
            }
            tries += 1;
            if (r1_D_001A74A8[r1_D_001A74C0] >= len) {
                break;
            }
        }
    }
    if (tries != 0 && r1_D_001A74F8 == 0) {
        func_00115D48(r1_D_001A78D0, tries);
    }
    if (flipped) {
        r1_D_001A74C4 = 1;
    }
    dst = (u8 *)(r1_D_001A74A0[r1_D_001A74C0]) - (r1_D_001A74A8[r1_D_001A74C0] - 0x1000);
    *(u16 *)dst = cmd;
    dst += 2;
    *(u16 *)dst = size;
    dst += 2;
    for (i = 0; i < size; i++) {
        dst[i] = data[i];
    }
    r1_D_001A74A8[r1_D_001A74C0] -= len;
    entry = (SndRingEntry *)(r1_D_001A74B0[r1_D_001A74C0] +
                             (*r1_D_001A74A0[r1_D_001A74C0] << 4));
    entry->field0 = arg3;
    entry->field8 = arg4;
    func_001330D0();
}
