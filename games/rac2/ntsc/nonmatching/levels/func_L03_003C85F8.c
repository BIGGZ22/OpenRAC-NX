/* 0 of 1 places match - last line: 03_endako: 96 bytes, expected 100
   Candidate is one instruction short.  Builds a mode record for the object's private block
   (retail's $s1), stores the helper's handle at +0xA0, fills the block's 0x90 slot from the
   caller's +0x10, then releases the object.  Retail carries one extra instruction: the
   caller's second argument is copied `$a1 -> $v0 -> $a0` (the copy waits in $v0 while $a0
   still holds the first argument), and retail gives the first argument $s0 where this cc1
   gives it $s1 (so the private block lands in $s1 vs $s0).  Tried and identical: the second
   argument parked in a local at entry, both pointers parked in locals at entry.  Needs the
   `$a1 -> $v0` hop, i.e. a source shape that keeps the copy live across the first argument's
   own use. */

typedef int s32;

extern s32 func_312898(s32 a0, s32 a1, void *a2);
extern void func_2EEFE0(char *a0, char *a1, void *a2);
extern void func_31B920(char *a0, s32 a1, s32 a2);

void func_L03_003C85F8(char *a0, s32 a1)
{
    char *v = a0;
    char *s1 = *(char **)(a0 + 0x68);
    char buf[0x10];
    s32 mode = a1;

    *(s32 *)(s1 + 0xA0) = func_312898(mode, 2, buf);
    func_2EEFE0(s1 + 0x90, v + 0x10, buf);
    func_31B920(v, 2, -1);
}
