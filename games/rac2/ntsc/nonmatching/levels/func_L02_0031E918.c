/* 0 of 8 places match - instruction at +0x1c differs; candidate is 88 bytes, size exact.
   Everything matches but which float register holds what: retail reads the value into $f0
   (`mtc1 $v0,$f0; cvt.s.w $f0,$f0`), the 1/4096 constant into $f1 and 1.0f into $f2, so the chain is
   `mul.s $f0,$f0,$f1; mul.s $f0,$f0,$f0; sub.s $f0,$f2,$f0`; this cc1 makes it $f1/$f2/$f0 and
   `mul.s $f1,$f1,$f2; mul.s $f1,$f1,$f1; sub.s $f0,$f0,$f1` - a cyclic permutation of the same
   three registers, every other instruction identical.
   Tried: the one-expression form and the three-local form (`unit`, `r`, `one`). Both give the same
   permutation. The callee is the boot function func_001163B0. */

float func_L02_0031E918(float scale)
{
    float r = (float)((func_001163B0() >> 16) & 0xFFF) * (1.0f / 4096.0f);

    return (1.0f - r * r) * scale;
}
