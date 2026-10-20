/* func_L08_00425328 (108 bytes, 1 place) - NOT C with this compiler: the success arm is one
 * instruction this cc1 cannot emit.
 *
 * Docker-free ruler (scratch/wslcmp.py): "08_tabora: 116 bytes, expected 108" for both
 * spellings tried.  Everything up to the branch is understood and reproduces retail:
 *   low[4] and high[4] are two 16-byte copies of *(L08Quad *)a0 (lq/sq), the third float of
 *   `low` is f13 (swc1 $f13,0x8($sp)) and of `high` is f12 (swc1 $f12,0x18($sp)), and the
 *   call is func_002761F8(high, low, 2, 0, 0).
 * Residue:
 *   (a) THE WALL: retail's success arm is the single word 0x4600A028 - COP1 fmt=S funct=0x28,
 *       fd=$f0 fs=$f0 ft=$f20, i.e. `f0 = D_L08_001BFF68 <op> f14` in one instruction.  Max
 *       semantics (the else arm returns f14 untouched) say it is max.s.
 *       Measured with this cc1 (the retail SDK build under WSL):
 *         `D > f14 ? D : f14`   -> lwc1 $f0 ; c.olt.s $f1,$f20 ; nop ; bc1f ; mov.s ; mov.s
 *         `r = D; if (r < f14) r = f14;` -> lwc1 $f0 ; c.olt.s $f0,$f20 ; nop ; bc1tl ; mov.s
 *       Both are 116 bytes: cc1 has no max.s/min.s pattern, so the one-instruction form is
 *       unreachable from C text.  The same word (0x46010028) is in func_L08_00425398, so that
 *       function is behind the same wall.
 *   (b) minor: this cc1 hoists the call's argument setup ($a2 = 2, $a3 = 0, $t0 = 0,
 *       $a1 = $sp) above the two copies; retail keeps it after them.
 *   (c) retail keeps the parameter in $v1 (`daddu $v1,$a0,$zero`) for the two lq's.
 *
 * To land either function, cc1 needs max.s (funct 0x28) as an opt-in patch, or the row needs
 * an inline-asm exception.  Nothing else is missing.
 */

typedef signed int s32;
typedef float f32;

typedef int L08Quad __attribute__((mode(TI)));

extern int func_002761F8(void *a, void *b, int mode, void *self, int flag);
extern f32 D_L08_001BFF68;

f32 func_L08_00425328(char *a0, f32 f12, f32 f13, f32 f14) {
    f32 low[4];
    f32 high[4];
    f32 r;

    *(L08Quad *)low = *(L08Quad *)a0;
    low[2] = f13;
    *(L08Quad *)high = *(L08Quad *)a0;
    high[2] = f12;
    if (func_002761F8(high, low, 2, 0, 0)) {
        r = D_L08_001BFF68;
        if (r < f14) {
            r = f14;
        }
        return r;
    }
    return f14;
}
