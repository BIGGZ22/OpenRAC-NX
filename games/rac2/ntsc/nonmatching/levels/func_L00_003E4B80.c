/* 0 of 27 exact: 30 instructions, 100 bytes; retail 108 (32 instructions).
 *
 * Everything but two nops is identical, including the early-out, the join
 * through $a0 and both calls:
 *   retail: mtc1 $s0,$f12 ; nop ; cvt.s.w $f12,$f12 ; nop ; nop ; div.s $f12,$f0,$f12
 *   ours:   mtc1 $s0,$f12 ; nop ; cvt.s.w $f12,$f12 ;               div.s $f12,$f0,$f12
 *
 * The missing pair sits at the cvt.s.w -> div.s division site, the same
 * compiler-side class documented in docs/COMPILER-NOTES.md (the scheduler
 * drops the two cvt.s.w latency nops; no C shape this lane tried restores
 * them). Same result with the divide operand hoisted into its own float
 * temporaries, with the cast written as 16.0f, and with the result of the
 * first call stored before the conversion.
 */
extern float func_00283CE0(void);

extern int func_00283CF0(float value);

int func_L00_003E4B80(int n)
{
    int unit;
    int k;
    int result;

    unit = 16;
    result = n;
    if (n % unit != 0) {
        k = func_00283CF0(func_00283CE0() / (float)unit);
        result = (k + 1) * unit;
    }
    return result;
}
