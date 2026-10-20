/* 0 of 1 places match - last line: 08_tabora: instruction at +0x8 differs
   Candidate is 184 bytes, retail's exact size, and the body is retail's: the three-slot
   walk with `i` in $a1 bumped in the `bnel` delay slot, the four pointers advanced at the
   loop bottom, the `slti $v0,$a1,3` back edge, and the found-slot arm that clears the 0x1B39A0
   word, releases each live handle through func_00291450 and answers 1 (the miss path answers
   0). THE PROLOGUE IS THE BLOCKER: retail materialises the four bases in the order
   0x1B39A0 -> $a2, 0x1B39E0 -> $s1, 0x1B39F0 -> $s2, 0x1B3A30 -> $v1, each `lui`/`addiu`
   pair interleaved with one `sd` of the save block, and keeps the walk's compare base in
   $v1. This cc1 groups them differently, keeps one base in $a0/$a3 and puts the walk's base
   elsewhere, which moves every following word.
   Three spellings measured, same interleave: pointers declared 0x1B39E0-first, pointers
   declared 0x1B39A0-first, and separate declarations with the assignments written out in
   retail's order. The sibling func_L08_00429408 (near miss, same data family) records the
   same wall with the array-index and pointer-local spellings. */
typedef signed int s32;

extern s32 D_L08_001B39A0[];
extern s32 D_L08_001B39E0[];
extern s32 D_L08_001B39F0[];
extern s32 D_L08_001B3A30[];
extern void func_00291450(s32 a0);

/* Frees the one of the three slots whose 0x1B3A30 word still names A0. */
s32 func_L08_00429350(s32 a0)
{
    s32 *pE0 = D_L08_001B39E0;
    s32 *pF0 = D_L08_001B39F0;
    s32 *pA0 = D_L08_001B39A0;
    s32 *p30 = D_L08_001B3A30;
    s32 i;
    s32 v;

    for (i = 0; i < 3; i++) {
        if (*p30 == a0) {
            v = *pE0;
            *p30 = 0;
            *pA0 = 0;
            if (v != -1) {
                func_00291450(v);
                *pE0 = -1;
            }
            v = *pF0;
            if (v != -1) {
                func_00291450(v);
                *pF0 = -1;
            }
            return 1;
        }
        pE0++;
        pF0++;
        pA0++;
        p30++;
    }
    return 0;
}
