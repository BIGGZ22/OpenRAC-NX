/* 0 of 1 places match - last line: 20_yeedil: 204 bytes, expected 208
   The whole ramp is recovered and every instruction is retail's. Two integer parameters
   feed the guards through inline `cvt.s.w` (`D_L20_001B20A0 <= (f32)a0` returns f12,
   `(f32)a1 <= D_L20_001B20A0` returns f13); the interpolated return calls the int-to-float
   helper func_00283CE0 three times - twice on a0, once on a1, in the order the inline
   denominator gives - and divides (D_L20_001B20A0 - h(a0)) by (h(a1) - h(a0)). Two
   residues: the `nop` retail keeps between the third `sub.s` and the `div.s` is absent, and
   one set-up is swapped at +0x8C (retail saves `$s1 = $a1` in the bc1f delay slot with
   `mov.s $f23,$f13` four words earlier; this cc1 the other way round). Named numerator and
   denominator locals give 200 bytes and a different schedule; spelling the three helpers as
   `(f32)` casts lets this cc1 fold them into the guards (104 bytes). 
   Second wave (20_yeedil, 2026-10-20) re-measured this one and adds three facts.
   (1) The symbol is D_L20_001B20B0, not D_L20_001B20A0: retail loads it with
   `lwc1 $f1,0x30B0($gp)` and gp is 0x1AEFF0, so the constant lives at 0x1B20B0 (the
   unit's -g8.c sibling already declares D_L20_001B20B0 and uses it). (2) That symbol is
   a FLOAT: declared as `int` the compiler adds a `cvt.s.w` right after the load, and
   changing the declaration in src/levels/20_yeedil/L20_0043ECB0-g8.c from int to float
   is what makes the load and the `c.ole.s` compare come out right - it does not disturb
   func_L20_00444C18, which only takes its address (still 1 of 1 places match).
    Third wave (20_yeedil, 2026-10-20, worker 04) re-measured this one in the -G8 sibling
    (the unit L20_0043ECB0-g8.c, built with -mno-split-addresses) and the picture is the
    same, 204 bytes against 208. Two facts worth keeping: the two `nop`s the retail keeps
    after the `mtc1` and after the `c.le.s` ARE emitted - they come from the assembler's
    mtc1 hazard rule, not from cc1 - so the only missing word is the `nop` before the
    `div.s` (the division floor of docs/COMPILER-NOTES.md: the assembler pads a division
    only when a code label precedes it, and no source shape puts one within two slots of
    the `div.s`; the division here is 15 slots after the block label). The other residue is
    the swap: retail keeps `$s1 = $a1` in the delay slot of the `bc1f` and `mov.s $f23,$f13`
    four words earlier, this cc1 has `mov.s $f23,$f13` in the delay slot and the copy in
    the prologue. Tried and not enough: the two guards with named locals for f12 and f13,
    an explicit copy of a1, both together, and the blend with the quotient in a named local
    (which gives 200 bytes and a different schedule).
   (3) The file has to be built with -G8 (the -g8.c sibling): at -G0 the load comes out
   `lui $at ; lwc1 $f1,0x0($at)`. The two residues are the same as recorded above: the
   `nop` retail keeps before the `div.s` (the load-delay nop that belongs to the
   `lwc1 $f1,0x30B0($gp)`), and the copy of the second argument into $s1 sits in the
   delay slot of the `bc1f` in retail while this cc1 puts it in the prologue block (and
   the `mov.s $f23,$f13` takes its place). Tried and not enough: the source spelled with
   if/else-if, with a result variable, with the three helper calls in named locals, with
   the numerator in a named local, and with the second comparison written as `>=` or as
   `!(a > b)` (those change the compare opcode and are wrong).*/
#include "common.h"

extern f32 D_L20_001B20A0 __attribute__((sda));

extern f32 func_00283CE0(s32);

/* The zero-crossing ramp between two integer positions: f12 at or below D_L20_001B20A0,
   f13 at or above it, and the linear blend of the two in between. */
f32 func_L20_00444B48(s32 a0, s32 a1, f32 f12, f32 f13)
{
    if (D_L20_001B20A0 <= (f32)a0)
        return f12;
    if ((f32)a1 <= D_L20_001B20A0)
        return f13;
    return f12 + (f13 - f12) *
                      ((D_L20_001B20A0 - func_00283CE0(a0)) /
                       (func_00283CE0(a1) - func_00283CE0(a0)));
}
