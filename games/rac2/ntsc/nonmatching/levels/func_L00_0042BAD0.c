/* family of four identical bodies: func_L00_0042BAD0 / _BCF8 / _BFF8 / _C290
   last check.py line: "00_aranos_tutorial: instruction at +0x18 differs"
   Only deviation: cc1 hoists ld $ra,0x8($sp) before the store and places daddu $v0,$s0,$zero
   between it and addiu; retail keeps lui / addiu / daddu / sw / ld $s0 / ld $ra.
   Variants tried and rejected: char *self = a0; same. Only the constant differs
   between the four (0x001ADD98 / 0x001ADD38 / 0x001ADD78 / 0x001ADD58). */
void *func_L00_0042BAD0(char *a0)
{
    func_L00_0042B8D8(a0);
    *(char **)(a0 + 0x30) = D_L00_001ADD98;
    return a0;
}
