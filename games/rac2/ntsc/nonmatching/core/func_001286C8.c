/* libdbc's send-side twin of sceDbcReceiveData (Lombyte MIT
   src/sdk/library/scedbcreceivedata.c, whose RPC buffer and payload layout it
   shares). 228 bytes against retail's 224: the two field stores and the
   payload loop each still come out right, but this cc1 materialises the buffer
   base twice - once in a temporary for unk4/unk8, once again for the payload -
   where retail builds `s1 = %hi` / `t0 = s1 + %lo` once and uses $t0 for both,
   and the base therefore lands in a caller-saved register instead of $t0.
   Retail also puts %hi in $s1 (saved first) where this build uses $s0.
   Tested: `s32 *state = r1_D_0015B180` (base in $s0, same 1-instruction
   excess, only the first store's form differs) and the all-symbolic
   `r1_D_0015B180.unk4 = arg1` form above. Unblocked by a source shape that
   makes this cc1 CSE the base across the payload loop. */
#include "common.h"

/* The send twin of sceDbcReceiveData (Lombyte MIT src/sdk/library/
   scedbcreceivedata.c): fill the libdbc RPC buffer with the caller's two
   header words and its count, copy the caller's bytes into the buffer's
   payload at +0xC, check the 0x8000091B socket, then run the send RPC and
   report failure with the recorded exit path.
   RAC2 addresses: the buffer r1_D_0015B180 is at 0x00143180 (its payload at
   +0xC) and the second DBC socket r1_D_0015B130 at 0x00143130; both are the
   Lombyte pair D_0015B080/D_0015B030 minus 0x17F00.
   The first header word goes through the symbol and the rest through the
   pointer, which is what puts retail's `sw $a0,0x3180($s1)` and then the
   `addiu $t0,$s1,0x3180` base in that order. */

struct DbCState {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 payload[0x80];
    s32 unk8C;
};

extern s32 r1_D_0015B130[];
extern struct DbCState r1_D_0015B180;

extern char D_0013B990[];

extern s32 func_0011D810();
extern s32 func_0011D620();
extern void func_00128898(void *, ...);

s32 func_001286C8(s32 arg0, s32 arg1, s32 *arg2, u8 *arg3) {
    s32 i;

    r1_D_0015B180.unk0 = arg0;
    r1_D_0015B180.unk4 = arg1;
    r1_D_0015B180.unk8 = *arg2;
    i = 0;
    if (*arg2 > 0) {
        do {
            r1_D_0015B180.payload[i] = arg3[i];
            i++;
        } while (i < *arg2);
    }
    if (func_0011D810(r1_D_0015B130) == 1) {
        return 0;
    }
    if (func_0011D620(r1_D_0015B130, 0x8000091B, 1, &r1_D_0015B180, 0x400, &r1_D_0015B180,
                     0x400, 0, 0) < 0) {
        func_00128898(D_0013B990);
        return 0;
    }
    return 1;
}
