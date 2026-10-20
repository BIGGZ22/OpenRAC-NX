/* 0 of 2 places match - last line: 04_barlow: 116 bytes, expected 124
   Candidate is two instructions short.  The walk over the 140 records of 0x150 bytes at
   0x00254E10 is right (a `do {} while` is required: retail has no pre-loop guard), but
   retail annuls both tests (`beql $v0,$zero` with the record advance in the delay slot,
   `bnel $v0,$zero` for the `< 2` test) and duplicates the advance into both paths, and its
   loop test is the SIGNED `slt` where this cc1 emits `sltu` for a pointer compare - so the
   pointer pair has to be carried as signed integers.  Retail also loads the record's 0x140
   word into $t0 without using it (a dead load this cc1 does not emit).  Tried and identical:
   `while (p < end)` (that form adds a pre-loop guard), `do {} while`, and the second test
   read into a named local.  Needs cc1 to annul the two branches. */

typedef int s32;
typedef unsigned char u8;

extern u8 D_L04_00254E10[];
extern void func_0030C948(u8 *a0, s32 a1, s32 a2, s32 a3);

void func_L04_00415E28(void) {
    s32 base = (s32)D_L04_00254E10;
    s32 end = base + 0xB7C0;
    s32 p;

    for (p = base; p < end; p += 0x150) {
        if (*(s32 *)(p + 0x144) == 0)
            continue;
        if (*(s32 *)(p + 0x138) < 2)
            continue;
        func_0030C948((u8 *)p, 0x43, 0x44, 0x45);
    }
}
