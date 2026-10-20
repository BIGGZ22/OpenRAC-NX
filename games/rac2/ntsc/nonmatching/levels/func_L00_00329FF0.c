/* 100 bytes against retail's 108 (27 places): every instruction matches but two FPU-latency
 * nops. Retail's sequence after the call is `mtc1 $s0,$f12; nop; cvt.s.w $f12,$f12; nop; nop;
 * div.s $f12,$f0,$f12` - this cc1 emits the nop after `mtc1` but not the pair after `cvt.s.w`,
 * and the 8 bytes are the whole difference. The body needs the REGISTER-divisor form: a literal
 * 16 folds the modulo into `andi $v0,$a0,0xF` and the division into `mul.s` by the reciprocal
 * (that candidate is 64 bytes); a local `s32 unit = 16` gives retail's `div $zero,$a0,$s0` and
 * `div.s`, and shares $s0 for the int-to-float conversion as retail does. Tried as well: the
 * -G2 and -G8 profiles and the SDK compiler (92 files in config/cflags.txt use `sdk -O2 -G2`);
 * all reach 100 bytes with the same two-nop gap.
 *
 * What would unblock it: an FPU hazard/padding control the repository does not have yet - the
 * same class as the FPU-allocation notes in nonmatching/levels/func_L00_003EE288.c.
 */
typedef int s32;

extern float func_002EF240(void);
extern int func_002EF250(float);

s32 func_L00_00329FF0(s32 value)
{
    s32 unit = 16;

    if (value % unit != 0)
        value = (func_002EF250(func_002EF240() / unit) + 1) * unit;
    return value;
}
