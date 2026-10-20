/* 0 of 2 places match - last line: 04_barlow: 56 bytes, expected 60
   Candidate is one instruction short.  Clears the 0x144 word of every one of the 140 records
   at 0x00254E10 whose 0x148 word names the argument, or all of them when the argument is 0.
   Size aside the sequence is right, but retail annuls both tests (`beql $a0,$zero` with the
   store in the delay slot, `bnel $v0,$a0`) where this cc1 emits `beqz`/`bne`, and retail
   finishes `addiu $a1,$a1,-1` twice (once in the `bnel` delay slot, once at the shared loop
   bottom) where this cc1 hoists the single decrement into the `beqz` delay slot - one word
   fewer.  Tried and identical: the `do {} while (--n >= 0)` form, a `for` with the decrement
   in the increment, the record pointer stepped as a byte pointer.  Needs cc1 to annul the
   two branches. */

typedef int s32;
typedef unsigned char u8;

extern u8 D_L04_00254E10[];

void func_L04_00415EA8(s32 a0) {
    s32 *p = (s32 *)(D_L04_00254E10 + 0x144);
    s32 n;

    for (n = 0x8B; n >= 0; n--) {
        if (a0 == 0)
            p[0] = 0;
        else if (p[1] == a0)
            p[0] = 0;
        p = (s32 *)((u8 *)p + 0x150);
    }
}
