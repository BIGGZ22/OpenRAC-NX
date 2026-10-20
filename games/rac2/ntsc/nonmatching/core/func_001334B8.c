/* from src/core/00133490.c: CdStartRead (symbol_addrs/usa 0x001334B8, size 0xC4).
   The file is one of the -G8 units (src/core/00133490-g8.c holds its sibling
   func_00133640). This candidate has the exact retail size (196) and matches byte for
   byte except one instruction: tools/check.py reports "instruction at +0x48 differs".
   Retail fills the `beq` delay slot with the return-0 constant
   (`daddu $v0,$zero,$zero` from the return-zero block) and branches to +168; this cc1
   hoists the first argument's `lui` into the slot instead, so the return-zero block
   stays between the two exits and an extra `b` appears at the end. Everything else -
   the two `lui $at` stores to 0x1A7100/0x1A7110, the two argument addresses as
   lui/addiu symbols, the payload stores and both calls - lines up. Unblocked by a
   source shape that makes this cc1 fill that delay slot from the branch target.

   WAVE 4 re-tries, all measured on this body (the file has to be its own *-g8.c:
   00133490-g8.c holds the already-matching sibling func_00133640):
     * the if/else shape with a shared exit - `if (func_00133580(1) == 1) ret = 0;
       else { body } return ret;` - puts the ENTIRE body in retail's exact order
       and position, but flips the test (bne to the body) and keeps the zero in an
       in-line block with `b epilogue`, so the same +0x48 word is still the only
       difference. Inverting that condition (`!= 1` with the body in the then-arm,
       with and without the shared return) reproduces the original layout instead.
       A plain `int ret; ret = 0;` before the test costs a callee-saved register
       and grows the frame: 204 bytes against 196.
     * `-fno-schedule-insns2` groups the prologue saves (sd $s0..$ra together) where
       retail interleaves them with the daddu moves, so the scheduler is part of the
       match - but the BODY ORDER is unchanged (lui $a0 first), so the hoist into the
       slot is not sched2. `-fno-schedule-insns` changes nothing at all.
     * passing the two addresses straight in the call (no p/q locals) changes nothing.
   What remains: the beq's slot must take the return-0 constant and the branch must
   be retargeted to +168 (the epilogue); this cc1 fills it from the fall-through
   (`lui $a0,0x1A`) and leaves the return-0 block in line with an extra `b`. */
#include "common.h"

extern int D_001A748C __attribute__((sda));

extern int D_001A7494 __attribute__((sda));

extern int D_001A7498 __attribute__((sda));

extern char r1_D_001A7100_end[];

extern char r1_D_001A713F_end[];

extern void func_0011B3D0(void *, void *);

extern int func_001253A8(void);

extern int func_00133580(int);

extern void func_00132DF0(int, int, void *, int, int);

int func_001334B8(int arg0, int arg1, int arg2) {
    int payload[4];
    char *p;
    char *q;

    if (D_001A748C == 0) {
        return func_001253A8();
    }
    if (func_00133580(1) == 1) {
        return 0;
    }
    *(int *)0x1A7100 = 1;
    p = r1_D_001A7100_end;
    q = r1_D_001A713F_end;
    *(int *)0x1A7110 = 0;
    func_0011B3D0(p, q);
    payload[0] = arg0;
    payload[1] = arg1;
    payload[2] = arg2;
    D_001A7494 = 1;
    D_001A7498 = 0;
    func_00132DF0(0x38, 0xC, payload, 0, 0);
    return 1;
}
